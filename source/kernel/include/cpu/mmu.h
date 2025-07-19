/**
 * MMU与分布处理
 */
#ifndef MMU_H
#define MMU_H

#include "comm/types.h"
#include "comm/cpu_instr.h"

#define PDE_CNT             1024
#define PTE_CNT             1024
#define PTE_P              (1 << 0)
#define PTE_W              (1 << 1)
#define PDE_P              (1 << 0)
#define PTE_U              (1 << 2)
#define PDE_U              (1 << 2)

#pragma pack(1)
/**
 * @brief Page-Directory Entry
 */
typedef union _pde_t {
    uint32_t v;
    struct {
        uint32_t present : 1;            // 第0位，页表是否存在
        uint32_t write_disable : 1;      // 第1位，是否只读（0: 可写，1: 只读）
        uint32_t user_mode_acc : 1;      // 第2位，是否允许用户态访问（1 允许，0 仅内核）
        uint32_t write_through : 1;      // 第3位，写穿透缓存
        uint32_t cache_disable : 1;      // 第4位，禁止缓存
        uint32_t accessed : 1;           // 第5位，CPU是否访问过（由CPU自动置位）
        uint32_t : 1;                    // 第6位，保留位
        uint32_t ps : 1;                 // 第7位，页大小（0=4KB，1=4MB大页）
        uint32_t : 4;                    // 第8~11位，保留位
        uint32_t phy_pt_addr : 20;       // 第12~31位，高20位，指向页表的物理地址 可以转换为 pte_num[]数组
    };
}pde_t;

/**
 * @brief Page-Table Entry
 */
typedef union _pte_t {
    uint32_t v;
    struct {
        uint32_t present : 1;           // 第0位，页是否存在
        uint32_t write_disable : 1;     // 第1位，是否可写
        uint32_t user_mode_acc : 1;     // 第2位，是否允许用户访问
        uint32_t write_through : 1;     // 第3位，缓存策略
        uint32_t cache_disable : 1;     // 第4位，是否禁止缓存
        uint32_t accessed : 1;          // 第5位，是否访问过
        uint32_t dirty : 1;             // 第6位，是否被写过（写操作置位）
        uint32_t pat : 1;               // 第7位，页属性表 PAT 位
        uint32_t global : 1;            // 第8位，是否全局页（G位）
        uint32_t : 3;                   // 第9~11位，保留
        uint32_t phy_page_addr : 20;    // 第12~31位，高20位，页的物理地址，可以转换为paddr[]数组
    };
}pte_t;

#pragma pack()

/**
 * @brief 返回vaddr在页目录中的索引
 */
static inline uint32_t pde_index (uint32_t vaddr) {
    int index = (vaddr >> 22); // 只取高10位
    return index;
}

/**
 * @brief 获取pde中地址
 */
static inline uint32_t pde_paddr (pde_t * pde) {
    return pde->phy_pt_addr << 12;
}

/**
 * @brief 返回vaddr在页表中的索引
 */
static inline int pte_index (uint32_t vaddr) {
    return (vaddr >> 12) & 0x3FF;   // 取中间10位
}

/**
 * @brief 获取pte中的物理地址
 */
static inline uint32_t pte_paddr (pte_t * pte) {
    return pte->phy_page_addr << 12;
}

/**
 * @brief 获取pte中的权限位
 */
static inline uint32_t get_pte_perm (pte_t * pte) {
    return (pte->v & 0x1FF);
}

/**
 * @brief 重新加载整个页表
 * @param vaddr 页表的虚拟地址
 */
static inline void mmu_set_page_dir (uint32_t paddr) {
    // 将虚拟地址转换为物理地址
    write_cr3(paddr);
}

#endif // MMU_H
