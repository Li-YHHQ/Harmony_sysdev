/* hw_cloud_entry.c — 华为云 MQTT 接入模块
 * 23001010613 李松伦
 *
 * 在 31_hw_cloud_demo/hw_cloud_entry.c 基础上扩展：
 *   - 上报：temperature / humidity / speed / motor_duty / motor_dir / safety_score
 *   - 命令：setBuzCmd / setMotorCmd / setTempLimit
 */

/* ——— 标准头文件 ——— */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cmsis_os2.h"
#include "ohos_init.h"
#include <cJSON.h>

/* ——— 扩展外设头文件 ——— */
#include "buzzer.h"     /* buzzer_pwm_start1 / buzzer_pwm_stop */
#include "led.h"        /* led_on / led_off */
#include "pwm_motor.h"  /* MOTOR_DIR_CW / MOTOR_DIR_CCW / pwm_motor_* */

/* ——— 华为云 MQTT 库 ——— */
#include <dtls_al.h>
#include <mqtt_al.h>
#include <oc_mqtt_al.h>
#include <oc_mqtt_profile.h>

/* ====================================================================
 * 华为云连接参数
 * ==================================================================== */
#define CONFIG_APP_SERVERIP   "117.78.5.125"  /* 标准版对接地址（不变） */
#define CONFIG_APP_SERVERPORT "1883"           /* MQTT接入端口（不变） */
#define CONFIG_APP_DEVICEID   "6a1a62facbb0cf6bb95febee_23001010613_lsl_pegasus" /* 设备ID */
#define CONFIG_APP_DEVICEPWD  "Lsl23001010613" /* 设备密钥 */
#define CONFIG_APP_LIFETIME   (60)             /* 心跳周期（秒） */
#define CONFIG_QUEUE_TIMEOUT  (5 * 1000)       /* 消息队列超时（ms） */

#define MSGQUEUE_COUNT        16
#define MSGQUEUE_SIZE         10
#define CLOUD_TASK_STACK_SIZE (1024 * 10)
#define CLOUD_TASK_PRIO       24

/* ====================================================================
 * 外部全局变量（由 main_entry.c 定义）
 * ==================================================================== */
extern float    g_temperature;
extern float    g_humidity;
extern float    g_temp_upLimit;
extern float    g_temp_doLimit;
extern int8_t   g_motor_duty;      /* 定义于 pwm_motor.c */
extern int8_t   g_motor_dir;       /* 定义于 pwm_motor.c，int8_t 匹配其定义类型 */
extern float    g_speed;
extern uint8_t  g_flag_buzzer_disable;
extern char     g_cloud_cmd_line1[32];
extern char     g_cloud_cmd_line2[32];
extern char     g_cloud_cmd_line3[32];

/* 安全评分计算函数（定义于 main_entry.c，此处声明供 deal_report_msg 调用） */
extern int calc_safety_score(void);

/* ====================================================================
 * 内部消息类型
 * ==================================================================== */
typedef enum {
    en_msg_cmd = 0, /* 华为云下发命令 */
    en_msg_report,  /* 向华为云上报数据（预留） */
} en_msg_type_t;

typedef struct {
    char *request_id;
    char *payload;
} cmd_t;

typedef struct {
    int lum; /* 预留字段 */
} report_t;

typedef struct {
    en_msg_type_t msg_type;
    union {
        cmd_t   cmd;
        report_t report;
    } msg;
} app_msg_t;

typedef struct {
    osMessageQueueId_t app_msg;
    int connected;
    int led;
} app_cb_t;

static app_cb_t g_app_cb;

/* ====================================================================
 * MQTT 消息接收回调：将云端命令推入消息队列
 * ==================================================================== */
static int msg_rcv_callback(oc_mqtt_profile_msgrcv_t *msg)
{
    int ret = 0;
    char *buf;
    int buf_len;
    app_msg_t *app_msg;

    if ((msg == NULL) || (msg->request_id == NULL) ||
        (msg->type != EN_OC_MQTT_PROFILE_MSG_TYPE_DOWN_COMMANDS)) {
        return ret;
    }

    buf_len = sizeof(app_msg_t) + strlen(msg->request_id) + 1 + msg->msg_len + 1;
    buf = malloc(buf_len);
    if (buf == NULL) {
        return ret;
    }

    app_msg = (app_msg_t *)buf;
    buf += sizeof(app_msg_t);

    app_msg->msg_type = en_msg_cmd;
    app_msg->msg.cmd.request_id = buf;
    buf_len = strlen(msg->request_id);
    memcpy_s(app_msg->msg.cmd.request_id, buf_len, msg->request_id, buf_len);
    app_msg->msg.cmd.request_id[buf_len] = '\0';
    buf += buf_len + 1;

    buf_len = msg->msg_len;
    app_msg->msg.cmd.payload = buf;
    memcpy_s(app_msg->msg.cmd.payload, buf_len, msg->msg, buf_len);
    app_msg->msg.cmd.payload[buf_len] = '\0';

    ret = osMessageQueuePut(g_app_cb.app_msg, &app_msg, 0U, CONFIG_QUEUE_TIMEOUT);
    if (ret != 0) {
        free(app_msg);
    }
    return ret;
}

/* ====================================================================
 * deal_report_msg — 上报多路传感器数据到华为云
 *
 * 属性链：
 *   temperature → humidity → speed → motor_duty → motor_dir → safety_score
 * ==================================================================== */
void deal_report_msg(void)
{
    if (g_app_cb.connected != 1) {
        return;
    }
    printf("23001010613 report to huawei cloud...\n");

    oc_mqtt_profile_service_t service;
    oc_mqtt_profile_kv_t Temp;
    oc_mqtt_profile_kv_t Humi;
    oc_mqtt_profile_kv_t Speed;
    oc_mqtt_profile_kv_t MotorDuty;
    oc_mqtt_profile_kv_t MotorDir;
    oc_mqtt_profile_kv_t Safety;    /* 方案B新增：安全评分 */

    /* motor_duty 为 int8_t，需转为 int 才能用 VALUE_INT */
    static int s_motor_duty_int;
    s_motor_duty_int = (int)g_motor_duty;

    /* motor_dir 上报字符串 */
    static char s_motor_dir_str[16];
    snprintf(s_motor_dir_str, sizeof(s_motor_dir_str), "%s",
             (g_motor_dir == MOTOR_DIR_CW) ? "Forward" : "Backward");

    /* 安全评分（每次上报时实时计算） */
    static int s_safety_score;
    s_safety_score = calc_safety_score();
    printf("23001010613 safety_score=%d\n", s_safety_score);

    /* 组装服务 */
    service.event_time       = NULL;
    service.service_id       = "23001010613_lsl_enviDetector"; /* 服务ID */
    service.service_property = &Temp;
    service.nxt              = NULL;

    /* 温度 */
    Temp.key   = "temperature";
    Temp.value = &g_temperature;
    Temp.type  = EN_OC_MQTT_PROFILE_VALUE_FLOAT;
    Temp.nxt   = &Humi;

    /* 湿度 */
    Humi.key   = "humidity";
    Humi.value = &g_humidity;
    Humi.type  = EN_OC_MQTT_PROFILE_VALUE_FLOAT;
    Humi.nxt   = &Speed;

    /* 车速 */
    Speed.key   = "speed";
    Speed.value = &g_speed;
    Speed.type  = EN_OC_MQTT_PROFILE_VALUE_FLOAT;
    Speed.nxt   = &MotorDuty;

    /* 电机占空比 */
    MotorDuty.key   = "motor_duty";
    MotorDuty.value = &s_motor_duty_int;
    MotorDuty.type  = EN_OC_MQTT_PROFILE_VALUE_INT;
    MotorDuty.nxt   = &MotorDir;

    /* 电机方向（字符串） */
    MotorDir.key   = "motor_dir";
    MotorDir.value = s_motor_dir_str;
    MotorDir.type  = EN_OC_MQTT_PROFILE_VALUE_STRING;
    MotorDir.nxt   = &Safety;      /* 指向安全评分节点 */

    /* 安全评分（方案B新增属性） */
    Safety.key   = "safety_score";
    Safety.value = &s_safety_score;
    Safety.type  = EN_OC_MQTT_PROFILE_VALUE_INT;
    Safety.nxt   = NULL;           /* 属性链末尾 */

    oc_mqtt_profile_propertyreport(NULL, &service);
}

/* ====================================================================
 * oc_cmdresp — 向华为云返回命令执行结果
 * ==================================================================== */
static void oc_cmdresp(cmd_t *cmd, int cmdret)
{
    oc_mqtt_profile_cmdresp_t cmdresp;
    cmdresp.paras      = NULL;
    cmdresp.request_id = cmd->request_id;
    cmdresp.ret_code   = cmdret;
    cmdresp.ret_name   = NULL;
    (void)oc_mqtt_profile_cmdresp(NULL, &cmdresp);
}

/* ====================================================================
 * deal_cmd_msg — 解析并执行华为云下发命令
 *
 * 支持三种命令：
 *   1. setBuzCmd   → 参数 OnOff("ON"/"OFF")
 *   2. setMotorCmd → 参数 dir("CW"/"CCW") + duty(int 0~100)
 *   3. setTempLimit→ 参数 high(double) + low(double)
 * ==================================================================== */
static void deal_cmd_msg(cmd_t *cmd)
{
    cJSON *obj_root    = NULL;
    cJSON *obj_cmdname = NULL;
    cJSON *obj_paras   = NULL;
    cJSON *obj_para    = NULL;
    int    cmdret      = 1; /* 默认失败 */

    /* 解析 JSON payload */
    obj_root = cJSON_Parse(cmd->payload);
    if (obj_root == NULL) {
        oc_cmdresp(cmd, cmdret);
        return;
    }

    /* 获取命令名称 */
    obj_cmdname = cJSON_GetObjectItem(obj_root, "command_name");
    if (obj_cmdname == NULL) {
        cJSON_Delete(obj_root);
        oc_cmdresp(cmd, cmdret);
        return;
    }

    /* ——— 命令1：setBuzCmd — 蜂鸣器使能/屏蔽 ——— */
    if (strcmp(cJSON_GetStringValue(obj_cmdname), "setBuzCmd") == 0) {
        obj_paras = cJSON_GetObjectItem(obj_root, "paras");
        if (obj_paras == NULL) {
            cJSON_Delete(obj_root);
            oc_cmdresp(cmd, cmdret);
            return;
        }
        obj_para = cJSON_GetObjectItem(obj_paras, "OnOff");
        if (obj_para == NULL) {
            cJSON_Delete(obj_root);
            oc_cmdresp(cmd, cmdret);
            return;
        }

        if (strcmp(cJSON_GetStringValue(obj_para), "ON") == 0) {
            /* ON → 解除屏蔽，蜂鸣器恢复温度超限逻辑 */
            g_flag_buzzer_disable = 0;
            snprintf(g_cloud_cmd_line1, sizeof(g_cloud_cmd_line1), "Buzzer cmd:Ena");
            printf("23001010613: setBuzCmd ON - 蜂鸣器使能\n");
        } else {
            /* OFF → 屏蔽蜂鸣器，立即停止 */
            g_flag_buzzer_disable = 1;
            buzzer_pwm_stop();
            snprintf(g_cloud_cmd_line1, sizeof(g_cloud_cmd_line1), "Buzzer cmd:Dis");
            printf("23001010613: setBuzCmd OFF - 蜂鸣器禁用\n");
        }
        cmdret = 0;
        oc_cmdresp(cmd, cmdret);

    /* ——— 命令2：setMotorCmd — 电机方向+占空比 ——— */
    } else if (strcmp(cJSON_GetStringValue(obj_cmdname), "setMotorCmd") == 0) {
        obj_paras = cJSON_GetObjectItem(obj_root, "paras");
        if (obj_paras == NULL) {
            cJSON_Delete(obj_root);
            oc_cmdresp(cmd, cmdret);
            return;
        }

        /* 解析方向参数 "dir"："CW" 或 "CCW" */
        cJSON *obj_dir  = cJSON_GetObjectItem(obj_paras, "dir");
        /* 解析占空比参数 "duty"：整数 0~100 */
        cJSON *obj_duty = cJSON_GetObjectItem(obj_paras, "duty");

        if (obj_dir == NULL || obj_duty == NULL) {
            cJSON_Delete(obj_root);
            oc_cmdresp(cmd, cmdret);
            return;
        }

        /* 转换方向 */
        MotorDir dir_val;
        if (strcmp(cJSON_GetStringValue(obj_dir), "CW") == 0) {
            dir_val = MOTOR_DIR_CW;
        } else {
            dir_val = MOTOR_DIR_CCW;
        }

        /* 转换占空比，并限幅到 [0,100] */
        int duty = (int)cJSON_GetNumberValue(obj_duty);
        if (duty < 0)   duty = 0;
        if (duty > 100) duty = 100;

        /* 更新全局状态 */
        g_motor_dir  = (int8_t)dir_val; /* g_motor_dir 是 int8_t，定义于 pwm_motor.c */
        g_motor_duty = (int8_t)duty;

        /* 应用到电机 */
        if (duty == 0) {
            pwm_motor_stop();
        } else {
            pwm_motor_start(dir_val, (int8_t)duty);
        }

        snprintf(g_cloud_cmd_line2, sizeof(g_cloud_cmd_line2),
                 "Motor cmd:%s %d", duty == 0 ? "stop" : "run", duty);
        printf("23001010613: setMotorCmd dir=%s duty=%d\n",
               dir_val == MOTOR_DIR_CW ? "CW" : "CCW", duty);

        cmdret = 0;
        oc_cmdresp(cmd, cmdret);

    /* ——— 命令3：setTempLimit — 设置温度上下限阈值 ——— */
    } else if (strcmp(cJSON_GetStringValue(obj_cmdname), "setTempLimit") == 0) {
        obj_paras = cJSON_GetObjectItem(obj_root, "paras");
        if (obj_paras == NULL) {
            cJSON_Delete(obj_root);
            oc_cmdresp(cmd, cmdret);
            return;
        }

        cJSON *obj_high = cJSON_GetObjectItem(obj_paras, "high");
        cJSON *obj_low  = cJSON_GetObjectItem(obj_paras, "low");

        if (obj_high == NULL || obj_low == NULL) {
            cJSON_Delete(obj_root);
            oc_cmdresp(cmd, cmdret);
            return;
        }

        float new_high = (float)cJSON_GetNumberValue(obj_high);
        float new_low  = (float)cJSON_GetNumberValue(obj_low);

        /* 确保 low < high */
        if (new_low < new_high) {
            g_temp_upLimit = new_high;
            g_temp_doLimit = new_low;
        } else {
            /* 参数非法时交换 */
            g_temp_upLimit = new_low;
            g_temp_doLimit = new_high;
        }

        snprintf(g_cloud_cmd_line3, sizeof(g_cloud_cmd_line3),
                 "TLimit H:%.0f L:%.0f", g_temp_upLimit, g_temp_doLimit);
        printf("23001010613: setTempLimit high=%.1f low=%.1f\n",
               g_temp_upLimit, g_temp_doLimit);

        cmdret = 0;
        oc_cmdresp(cmd, cmdret);
    }

    cJSON_Delete(obj_root);
}

/* ====================================================================
 * CloudCmdTask — 从消息队列取命令并处理
 * ==================================================================== */
static int CloudCmdTask(void)
{
    app_msg_t *app_msg;
    printf("[CloudCmdTask] started\n");

    while (1) {
        app_msg = NULL;
        (void)osMessageQueueGet(g_app_cb.app_msg, (void **)&app_msg,
                                NULL, 0xFFFFFFFF);
        if (app_msg != NULL) {
            switch (app_msg->msg_type) {
                case en_msg_cmd:
                    deal_cmd_msg(&app_msg->msg.cmd);
                    break;
                default:
                    break;
            }
            free(app_msg);
        }
    }
    return 0;
}

static void CloudCmdTaskEntry(void)
{
    osThreadAttr_t attr;
    attr.name       = "CloudCmdTask";
    attr.attr_bits  = 0U;
    attr.cb_mem     = NULL;
    attr.cb_size    = 0U;
    attr.stack_mem  = NULL;
    attr.stack_size = CLOUD_TASK_STACK_SIZE;
    attr.priority   = CLOUD_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)CloudCmdTask, NULL, &attr) == NULL) {
        printf("[CloudCmdTaskEntry] create CloudCmdTask failed\n");
    }
}

/* ====================================================================
 * huawei_cloud_mqtt_init — 连接华为云并启动命令接收线程
 * ==================================================================== */
int huawei_cloud_mqtt_init(void)
{
    printf("23001010613 李松伦 华为云IoT Demo Start!\n");

    uint32_t ret;
    dtls_al_init();
    mqtt_al_init();
    oc_mqtt_init();

    /* 创建消息队列（接收云端命令） */
    g_app_cb.app_msg = osMessageQueueNew(MSGQUEUE_COUNT, MSGQUEUE_SIZE, NULL);
    if (g_app_cb.app_msg == NULL) {
        printf("huawei_cloud_mqtt_init: create msg queue failed\n");
    }

    /* 配置连接参数 */
    oc_mqtt_profile_connect_t connect_para;
    (void)memset_s(&connect_para, sizeof(connect_para), 0, sizeof(connect_para));

    connect_para.boostrap      = 0;
    connect_para.device_id     = CONFIG_APP_DEVICEID;
    connect_para.device_passwd = CONFIG_APP_DEVICEPWD;
    connect_para.server_addr   = CONFIG_APP_SERVERIP;
    connect_para.server_port   = CONFIG_APP_SERVERPORT;
    connect_para.life_time     = CONFIG_APP_LIFETIME;
    connect_para.rcvfunc       = msg_rcv_callback;
    connect_para.security.type = EN_DTLS_AL_SECURITY_TYPE_NONE;

    /* 发起连接 */
    ret = oc_mqtt_profile_connect(&connect_para);
    if (ret == (int)en_oc_mqtt_err_ok) {
        g_app_cb.connected = 1;
        printf("huawei_cloud_mqtt_init: connect OK\n");
    } else {
        printf("huawei_cloud_mqtt_init: connect FAILED (ret=%u)\n", ret);
    }

    /* 启动命令接收线程 */
    CloudCmdTaskEntry();

    return 0;
}
