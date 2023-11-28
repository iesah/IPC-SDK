#ifndef __JZSSI__
#define __JZSSI__

struct jz47xx_spi_info {
	u8	chnl;				/* the chanel of SSI controller */
	u16	bus_num;			/* spi_master.bus_num */
	unsigned is_pllclk:1;			/* source clock: 1---pllclk;0---exclk */
	unsigned long	max_clk;
	unsigned long	board_size;		/* spi_master.num_chipselect */
	struct spi_board_info	*board_info; 	/* link to spi devices info */
	u32	 num_chipselect;
	unsigned int chipselect[2];

	void (*set_cs)(struct jz47xx_spi_info *spi, u8 cs,unsigned int pol); /* be defined by spi devices driver user */
	void (*pins_config)(void);		/* configure spi function pins (CLK,DR,RT) by user if need. */
};

#endif
