#ifndef _JZCPM_PWC_H_
#define _JZCPM_PWC_H_

#define PWC_X2D  "x2d"
#define PWC_VPU  "vpu"

int cpm_pwc_enable(void *handle);
int cpm_pwc_disable(void *handle);
int cpm_pwc_is_enabled(void *handle);
void *cpm_pwc_get(char *name);
void cpm_pwc_put(void *handle);
void cpm_pwc_init(void);
#endif /* _JZCPM_PWC_H_ */
