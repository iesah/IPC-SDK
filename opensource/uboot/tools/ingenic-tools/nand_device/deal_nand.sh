#!/bin/bash

nand_param="nand_param.h"

creat_nandparam_file() {
    print_head=$(echo $nand_param | tr 'a-z' 'A-Z')
    print_head=${print_head//'.'/'_'}

    if [ ! -f "$nand_param" ]
    then
	    touch $nand_param
    fi
#add file head
    echo "#ifndef __"$print_head"" > $nand_param
    echo "#define __"$print_head"" >> $nand_param
}


main() {
	creat_nandparam_file
	files=`ls *.c | grep "nand"`
# add function declare
	for file in $files
	do
		file=${file%.*}
		echo "int "$file"_register_func(void);" >> $nand_param
	done

#add function point
	echo "static void *nand_param[] = {" >> $nand_param
	for file in $files
	do
		file=${file%.*}
		echo "/*##################*/" >> $nand_param
		echo "(void *)"$file"_register_func," >> $nand_param
		echo "/*##################*/" >> $nand_param
	done

	echo "};" >> $nand_param

#add file end
	echo "#endif" >> $nand_param

}

main
