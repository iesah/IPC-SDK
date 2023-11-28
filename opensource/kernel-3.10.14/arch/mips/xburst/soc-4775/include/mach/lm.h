#ifndef __LM_H__
#define __LM_H__

struct lm_device {
	struct device		dev;
	struct resource		resource;
	unsigned int		irq;
	unsigned int		id;
	unsigned int 		driver_data;
};

struct lm_driver {
	struct device_driver	drv;
	int			(*probe)(struct lm_device *);
	void			(*remove)(struct lm_device *);
	int			(*suspend)(struct lm_device *, pm_message_t);
	int			(*resume)(struct lm_device *);
};

int lm_driver_register(struct lm_driver *drv);
void lm_driver_unregister(struct lm_driver *drv);

int lm_device_register(struct lm_device *dev);

#define lm_get_drvdata(lm)	((void*)(lm->driver_data))
#define lm_set_drvdata(lm,d)	do{lm->driver_data = (unsigned int)(d);}while(0)

#endif
