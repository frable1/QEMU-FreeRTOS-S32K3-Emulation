#ifndef S32K3X8_MPU_H
#define S32K3X8_MPU_H


void MPU_init(void);
void MPU_selftest(void);
void MPU_fault_test(void);

/* MPU access permissions (AP) */
#define MPU_AP_NO          (0U << 24)  /* No access */
#define MPU_AP_RW          (3U << 24)  /* Priv RW, Unpriv RW (your old name was MPU_AP_RW) */
#define MPU_AP_RO          (6U << 24)  /* Priv RO, Unpriv RO (your old name was MPU_AP_RO) */

/* Size field helper (region size = 2^(n+1) bytes) */
#ifndef MPU_RASR_SIZE_FIELD
#define MPU_RASR_SIZE_FIELD(n) (((n)&0x1FU) << 1)
#endif

#ifndef MPU_RASR_VALUE
#define MPU_RASR_VALUE(n, ap)  (1U | MPU_RASR_SIZE_FIELD(n) | (ap))
#endif

/* Handy sizes (n = log2(size) - 1) */
#ifndef SIZE_32KB
#define SIZE_32KB  14U
#endif

/* Forced fault test API */
void MPU_forced_fault(void);

#endif /* S32K3X8_MPU_H */
