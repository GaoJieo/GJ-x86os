/**
 * 键盘设备处理
 */
#include "comm/cpu_instr.h"
#include "cpu/irq.h"
#include "dev/kbd.h"
#include "tools/log.h"
#include "tools/klib.h"

static kbd_state_t kbd_state;	// 键盘状态

/**
 * 键盘映射表，分3类
 * normal是没有shift键按下，或者没有numlock按下时默认的键值
 * func是按下shift或者numlock按下时的键值
 * esc是以esc开头的的键值
 */
static const key_map_t map_table[256] = {
        [0x2] = {'1', '!'},
        [0x3] = {'2', '@'},
        [0x4] = {'3', '#'},
        [0x5] = {'4', '$'},
        [0x6] = {'5', '%'},
        [0x7] = {'6', '^'},
        [0x08] = {'7', '&'},
        [0x09] = {'8', '*' },
        [0x0A] = {'9', '('},
        [0x0B] = {'0', ')'},
        [0x0C] = {'-', '_'},
        [0x0D] = {'=', '+'},
        [0x0E] = {'\b', '\b'},
        [0x0F] = {'\t', '\t'},
        [0x10] = {'q', 'Q'},
        [0x11] = {'w', 'W'},
        [0x12] = {'e', 'E'},
        [0x13] = {'r', 'R'},
        [0x14] = {'t', 'T'},
        [0x15] = {'y', 'Y'},
        [0x16] = {'u', 'U'},
        [0x17] = {'i', 'I'},
        [0x18] = {'o', 'O'},
        [0x19] = {'p', 'P'},
        [0x1A] = {'[', '{'},
        [0x1B] = {']', '}'},
        [0x1C] = {'\n', '\n'},
        [0x1E] = {'a', 'A'},
        [0x1F] = {'s', 'B'},
        [0x20] = {'d',  'D'},
        [0x21] = {'f', 'F'},
        [0x22] = {'g', 'G'},
        [0x23] = {'h', 'H'},
        [0x24] = {'j', 'J'},
        [0x25] = {'k', 'K'},
        [0x26] = {'l', 'L'},
        [0x27] = {';', ':'},
        [0x28] = {'\'', '"'},
        [0x29] = {'`', '~'},
        [0x2B] = {'\\', '|'},
        [0x2C] = {'z', 'Z'},
        [0x2D] = {'x', 'X'},
        [0x2E] = {'c', 'C'},
        [0x2F] = {'v', 'V'},
        [0x30] = {'b', 'B'},
        [0x31] = {'n', 'N'},
        [0x32] = {'m', 'M'},
        [0x33] = {',', '<'},
        [0x34] = {'.', '>'},
        [0x35] = {'/', '?'},
        [0x39] = {' ', ' '},
};

static inline char get_key(uint8_t key_code) {
    return key_code & 0x7F;
}

static inline int is_make_code(uint8_t key_code) {
    return !(key_code & 0x80);
}

/**
 * 等待可写数据
 */
void kbd_wait_send_ready(void) {
    uint32_t time_out = 100000; 
    while (time_out--) {
        if ((inb(KBD_PORT_STAT) & KBD_STAT_SEND_FULL) == 0) {
            return;
        }
    }
}

/**
 * 向键盘端口写数据
 */
void kbd_write(uint8_t port, uint8_t data) {
    kbd_wait_send_ready();
    outb(port, data);
}

/**
 * 等待可用的键盘数据
 */
void kbd_wait_recv_ready(void) {
    uint32_t time_out = 100000;
    while (time_out--) {
        if (inb(KBD_PORT_STAT) & KBD_STAT_RECV_READY) {
            return;
        }
    }
}

/**
 * 读键盘数据
 */
uint8_t kbd_read(void) {
    kbd_wait_recv_ready();
    return inb(KBD_PORT_DATA);
}

/**
 * 更新键盘上状态指示灯
 */
static void update_led_status (void) {
    int data = 0;

    data = (kbd_state.caps_lock ? 1 : 0) << 0;
    kbd_write(KBD_PORT_DATA, KBD_CMD_RW_LED);
    kbd_write(KBD_PORT_DATA, data);
    kbd_read();
}

/**
 * 处理单字符的标准键
 */
static void do_normal_key (uint8_t raw_code) {
    char key = get_key(raw_code);		// 去掉最高位
    int is_make = is_make_code(raw_code);

    // 暂时只处理按键按下
	switch (key) {
	    // shift, alt, ctrl键，记录标志位
	case KEY_RSHIFT:
		kbd_state.rshift_press = is_make;  // 仅设置标志位
		break;
	case KEY_LSHIFT:
		kbd_state.lshift_press = is_make;  // 仅设置标志位
		break;
    case KEY_CAPS:  // 大小写键，设置大小写状态
		if (is_make) {
			kbd_state.caps_lock = ~kbd_state.caps_lock;
			update_led_status();
		}
		break;
    default:
        if (is_make) {
            // 根据shift控制取相应的字符，这里有进行大小写转换或者shif转换
            if (kbd_state.rshift_press || kbd_state.lshift_press) {
                key = map_table[key].func;  // 第2功能
            }else {
                key = map_table[key].normal;  // 第1功能
            }

            // 根据caps再进行一次字母的大小写转换
            if (kbd_state.caps_lock) {
                if ((key >= 'A') && (key <= 'Z')) {
                    // 大写转小写
                    key = key - 'A' + 'a';
                } else if ((key >= 'a') && (key <= 'z')) {
                    // 小写转大小
                    key = key - 'a' + 'A';
                }
            }

            // 最后，不管是否是控制字符，都会被写入
            log_printf("key=%c", key);
        }
        break;
    }
}

/**
 * @brief 按键中断处理程序
 */
void do_handler_kbd(exception_frame_t *frame) {
	// 检查是否有数据，无数据则退出
	uint8_t status = inb(KBD_PORT_STAT);
	if (!(status & KBD_STAT_RECV_READY)) {
        pic_send_eoi(IRQ1_KEYBOARD);
		return;
	}

	// 读取键值
    uint8_t raw_code = inb(KBD_PORT_DATA);
    do_normal_key(raw_code);

	// 读取完成之后，就可以发EOI，方便后续继续响应键盘中断
	// 否则,键值的处理过程可能略长，将导致中断响应延迟
    pic_send_eoi(IRQ1_KEYBOARD);
}

/**
 * 键盘硬件初始化
 */
void kbd_init(void) {
    update_led_status();

    irq_install(IRQ1_KEYBOARD, (irq_handler_t)exception_handler_kbd);
    irq_enable(IRQ1_KEYBOARD);
}
