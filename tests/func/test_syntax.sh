#!/usr/bin/env bash
tmpn=test_syntax_file
extra_opts=(-S syn_test,$tmpn,4,0 -s foo,,258,3 $tmpn)
macro='M-c f o o'
declare -A expected
expected[style_fg]='\bfg=258\b'
expected[style_bg]='\bbg=3\b'
source 'test.sh'

tmpn=test_syntax_file
extra_opts=(-S syn_test,$tmpn,4,0 -s foo,,1,2 $tmpn)
macro='M-c { enter f o o ; enter } enter M-\ M-a M-/ M-. M-a'
declare -A expected
expected[outdent_style]='<ch=102 fg=1 bg=2><ch=111 fg=1 bg=2><ch=111 fg=1 bg=2><ch=59 fg=0 bg=0>'
source 'test.sh'

on_exit() { [ -n "$tmpf" ] && rm -f $tmpf; }
trap on_exit EXIT
tmpf=$(mktemp 'mle.test_syntax.XXXXXXXXXX')

cat >$tmpf <<"EOD"
"aaa"
"aaa\\"
"aaa \" aaa"
"aaa"          bbb "aaa"
"aaa \" aaa"   bbb "aaa \" aaa"
"aaa \" aaa\\" bbb "aaa \" aaa\\"
"aaa"          bbb
"aaa\\"        bbb
"aaa \" aaa"   bbb
"aaa"          bbb "aaa"          bbb
"aaa \" aaa"   bbb "aaa \" aaa"   bbb
"aaa \" aaa\\" bbb "aaa \" aaa\\" bbb
bbb "aaa"          bbb
bbb "aaa\\"        bbb
bbb "aaa \" aaa"   bbb
bbb "aaa"          bbb "aaa"          bbb
bbb "aaa \" aaa"   bbb "aaa \" aaa"   bbb
bbb "aaa \" aaa\\" bbb "aaa \" aaa\\" bbb
EOD
extra_opts=(-yy $tmpf)
macro=''
declare -A expected
declare -A not_expected
# a==97 b==98
expected[style_dq_a]='<ch=97 fg=260\b'
expected[style_dq_b]='<ch=98 fg=0\b'
not_expected[style_dq_not_a]='<ch=97 fg=0\b'
not_expected[style_dq_not_b]='<ch=98 fg=260\b'
source 'test.sh'

cat >$tmpf <<"EOD"
"aa"         /*bb*/
"aa/*aa"     /*bb*/
"aa/*aa*/aa" /*bb*/
"aa"         /*bb*/
"aa/*aa"     /*bb"bb*/
"aa/*aa*/aa" /*bb"bb"bb*/
/*bb*/       "aa"
/*bb*/       "aa/*aa"
/*bb*/       "aa/*aa*/aa"
/*bb*/       "aa"
/*bb"bb*/    "aa/*aa"
/*bb"bb"bb*/ "aa/*aa*/aa"
EOD
extra_opts=(-yy $tmpf)
macro=''
declare -A expected
declare -A not_expected
# a==97 b==98
expected[style_sc_a]='<ch=97 fg=260\b'
expected[style_sc_b]='<ch=98 fg=7\b'
not_expected[style_sc_not_a]='<ch=97 fg=(0|7)\b'
not_expected[style_sc_not_b]='<ch=98 fg=(0|260)\b'
source 'test.sh'
