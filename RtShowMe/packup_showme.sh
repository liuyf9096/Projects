#!/bin/bash

cd /reetoo
mkdir rtshowme-temp
cd ./rtshowme
tar -cvzf showme_setup.tar.gz *
mv showme_setup.tar.gz ../showme_setup.tar.gz
cd ../
rm -r rtshowme-temp
