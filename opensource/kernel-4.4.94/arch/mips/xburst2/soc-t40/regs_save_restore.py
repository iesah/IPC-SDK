#!/usr/bin/env python
import os
import string

common_regs=["s0","s1","s2","s3",
        "s4","s5","s6","s7",
        "gp","sp","fp","ra"]
cp0_regs=[
        ["CP0_PAGEMASK","$5","0"],
        ["CP0_TLB_SPEC","$5","4"],
        ["CP0_STATUS","$12","0"],
        ["CP0_INTCTL","$12","1"],
        ["CP0_CAUSE","$13","0"],
        ["CP0_EBASE","$15","1"],
        ["CP0_CONFIG","$16","0"],
        ["CP0_CONFIG1","$16","1"],
        ["CP0_CONFIG2","$16","2"],
        ["CP0_CONFIG3","$16","3"],
        ["CP0_CONFIG4", "$16", "4"],
        ["CP0_CONFIG5", "$16", "5"],
        ["CP0_CONFIG7","$16","7"],
        ["CP0_LLADDR","$17","0"],
        ["PMON_CSR","$17","7"],
        ["PMON_HIGH","$17","4"],
        ["PMON_LC","$17","5"],
        ["PMON_RC","$17","6"],
        ["CP0_WATCHLo","$18","0"],
        ["CP0_WATCHHI","$19","0"],
        ["CP0_ERRCTL","$26","0"],
        ["CP0_CONTEXT", "$4", "0"],
        ]
def gen_get_array_size():
    size=0
    size+=len(common_regs)
    size+=len(cp0_regs)
    return size

def gen_print_head_and_macro():
    print("#include <asm/asm.h>")
    print("#include <asm/regdef.h>")
    print("")
    for s in cp0_regs:
        print("#define %s\t%s,%s" %(s[0],s[1],s[2]))
    print("")
def gen_define_global_arg(str, num):
    print("\t.data")
    print("\t.global %s" %(str))
    print("%s:" %(str))
    print("\t.align 5")
    print("\t.space %s,0" %(num))
    print("")

def gen_save_common_regs(index):
    for s in common_regs:
        print("\tsw\t%s,%d(k0)" % (s,index))
        index=index+4
    return index
def gen_restore_common_regs(index):
    for s in common_regs:
        print("\tlw\t%s,%d(k0)" % (s,index))
        index=index+4
    return index
def gen_save_cp0_regs(index):
    for s in cp0_regs:
        print("\tmfc0\tk1,%s" % (s[0]))
        print("\tsw\tk1,%d(k0)" % (index))
        index=index+4
    return index
def gen_restore_cp0_regs(index):
    for s in cp0_regs:
        print("\tlw\tk1,%d(k0)" % (index))
        print("\tmtc0\tk1,%s" % (s[0]))
        index=index+4
    return index
def gen_print_function(f_name, regs_stack):
    index=0
    print("\t.text")
    print("\t.global %s" %(f_name))
    print("\t.align 2")
    print("\t.ent %s,0" %(f_name))
    print("%s:" %(f_name))
    print("\t.set\tpush")
    print("\t.set\tnoreorder")
    print("\t.set\tnoat")
    print("")
    print("\tla\tk0, %s" %(regs_stack))
    print("\tsllv\tk0, k0, a2")
    '''
    save_goto(func, args, cpu)
    '''
    if f_name == "save_goto":
        index = gen_save_common_regs(index)
        print("")
        index = gen_save_cp0_regs(index)
        print("")
	print("\tmove\tt0, a0");
	print("\tmove\ta0, a1");
        print("\tjr.hb\tt0")
        print("\tnop")
    elif f_name == "restore_goto":
        index = gen_restore_common_regs(index)
        print("")
        index = gen_restore_cp0_regs(index)
        print("")
        print("\tjr.hb\tra")
        print("\tnop")
    else:
        print("not support")
    print("")
    print("\t.set\tat")
    print("\t.set\treorder")
    print("\t.set\tpop")
    print("\tEND(%s)" %(f_name))


def main():
    regs_size=gen_get_array_size()
    regs_size*=4
    nr_cpu = 2
    regs_size = nr_cpu * regs_size;

    gen_print_head_and_macro()
    gen_define_global_arg("_regs_stack", regs_size)
    gen_print_function("save_goto", "_regs_stack")
    gen_print_function("restore_goto", "_regs_stack")
    pass

if __name__ == '__main__':
    main()
