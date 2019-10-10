set ARGYLL_CREATE_WRONG_VON_KRIES_OUTPUT_CLASS_REL_WP=1
txt2ti3 -v %1.txt %1
colprof -v -r .3 -qh -D %1.icm -O %1.icm %1
txt2ti3 -v %1_adj.txt %1_adj
colprof -v -r .3 -qh -D %1_adj.icm -O %1_adj.icm %1_adj

