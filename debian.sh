#!/bin/bash
set -euxo pipefail

keyid='as@php.net'

die() { echo "$@" >&2; exit 1; }

sudo apt install -y \
    dh-make \
    devscripts \
    pbuilder \
    devscripts \
    lintian \
    $(awk '/Build-Depends/{$1=$2=$3=$4=""; print $0}' debian/control | tr -d ',')

{ gpg --list-keys | grep -q $keyid; } || die "Missing gpg key for $keyid"
[ -f /var/cache/pbuilder/base.tgz ] || sudo pbuilder create --debootstrapopts --variant=buildd

vers_maj_min_rel=$(head -n1 debian/changelog | awk '{print $2}' | tr -d '()')
vers_maj_min=$(echo $vers_maj_min_rel | cut -d- -f1)
tmpdir=$(mktemp -d --tmpdir 'mle.XXXXXXXXXX')
srcdir="$tmpdir/mle-$vers_maj_min"
mkdir $srcdir
cp -aT $(pwd) $srcdir
pushd $srcdir

git clean -fdx
git submodule foreach 'rm -rf *'
find . -name '.git*' | xargs rm -rf
rm debian.sh
rm -rf vendor
mkdir vendor
echo 'clean:' >vendor/Makefile

dh_make -s -y -c apache --createorig || true
debuild -i -us -uc -sa -S
debsign -k $keyid ../mle*.dsc
sudo pbuilder --build ../mle*.dsc

debsign -k $keyid --re-sign ../*.changes
lintian -i -I --show-overrides ../*.changes

popd

ls -l $tmpdir

# dput /tmp/.../*.changes
