#!/bin/bash

 mle_tname='foreach-awk'
 mle_stdin=$(echo -en "mle1 aaa \n nope \n mle3")
mle_expect=$(echo -en "MLE1 AAA \n nope \n MLE3")
read -r -d '' mle_lel <<'EOF'
x/mle/|`awk '{print toupper($0)}'`
EOF
source test.sh

 mle_tname='foreach-s-num'
 mle_stdin=$(echo -en "mle99 \n nope00 \n mle01")
mle_expect=$(echo -en "mlenum \n nope00 \n mlenum")
   mle_lel='x/mle/s/\d+/num/'
source test.sh

 mle_tname='foreach-a-i'
 mle_stdin=$(echo -en "one\ntwo")
mle_expect=$(echo -en "hi one bye\nhi two bye")
   mle_lel='L{ ^ a/hi / $ a/ bye/ }'
source test.sh

 mle_tname='foreach-proto'
read -r -d '' mle_stdin <<'EOF'
#include <fake.h>
//proto-start
//proto-end
static int a() {
}
static int b() {
}
static int c() {
}
EOF
read -r -d '' mle_expect<<'EOF'
#include <fake.h>
//proto-start
static int a();
static int b();
static int c();
//proto-end
static int a() {
}
static int b() {
}
static int c() {
}
EOF
   mle_lel='x/^static/Y g /proto-start/ $ #+1 v g /proto-start/ D /proto-end/ s/ {/;/ U'
source test.sh
