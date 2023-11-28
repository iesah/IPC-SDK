#!/bin/sh

SRCDIR=$1
DST_ROOTFSNAME=$2
SIZE=$3
NOR_ERASE_SIZE=$4
NOR_PAGE_SIZE=$5
OUT=$PWD/$DST_ROOTFSNAME

if [ $# -ge 3 ]; then

	if [ $# -eq 5 ]; then
		ERASE_SIZE=`echo $NOR_ERASE_SIZE | awk -F 'K' '{print $1}'`
		ERASE_SIZE_10=`expr $ERASE_SIZE \* 1024`
		ERASE_SIZE_16=`echo "obase=16;$ERASE_SIZE_10"|bc`

		PAGE_SIZE=`echo $NOR_PAGE_SIZE | awk -F 'K' '{print $1}'`
		PAGE_SIZE_10=`expr $PAGE_SIZE \* 1024`
		PAGE_SIZE_16=`echo "obase=16;$PAGE_SIZE_10"|bc`
	else
		ERASE_SIZE_16=
		PAGE_SIZE_16=
	fi

	if [ -f $OUT ]; then
		rm $OUT
	fi

	find $SRCDIR -name .gitignore | xargs rm -vf

	mksquashfs $SRCDIR $OUT -comp xz

	TOTAL=`ls -l $OUT | awk -F ' ' '{print $5}'`
	TOTAL_KB=`expr $TOTAL / 1024`
	
	FS_SIZE=`echo $SIZE | awk -F 'K' '{print $1}'`
	FS_SIZE_10=`expr $FS_SIZE \* 1024`

	echo "============  The real size of $DST_ROOTFSNAME is $TOTAL_KB KB ============="

:<<OFF
	if [ $FS_SIZE_10 -gt $TOTAL ]; then
		ADD_SIZE=`expr $FS_SIZE_10 - $TOTAL`
		#echo "ADD_SIZE=$ADD_SIZE"
		tr '\000' '\377' < /dev/zero | dd of=add bs=1 count=$ADD_SIZE
		cat add >> $OUT
		rm add
		ls -lh $OUT
		exit 0
	else
		rm $OUT
		echo "----------- the inputed size of squashfs is too little ---------"
		exit 1
	fi
OFF
else
	echo "./mksquashfs.sh srcdir dstdir/imgname img_size [nor_erase_size] [nor_page_size]"
	exit 1
fi
