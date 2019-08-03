#!/bin/bash

if [ $# = 0 ];
then
echo "Estaba esperando 1 parametro para el numero de memoria y un opcional para el ip"
exit 1
fi

IP_MEM=127.0.0.1
IP_LFS=127.0.0.1
IP_MEM3=127.0.0.1
IP_MEM4=127.0.0.1
IP_MEM1=127.0.0.1

if [ $# -eq 2 ];
then
IP_MEM=$2
fi

if [ $# -eq 3 ];
then
IP_MEM=$2
IP_LFS=$3
fi

if [ $# -eq 4 ] && [ $1 -eq 1 ];
then
IP_MEM=$2
IP_LFS=$3
IP_MEM3=$4
fi

if [ $# -eq 4 ] && [ $1 -eq 2 ];
then
IP_MEM=$2
IP_LFS=$3
IP_MEM4=$4
fi

if [ $# -eq 4 ] && [ $1 -eq 3 ];
then
echo "La memoria 3 no tiene seeds"
exit 1
fi

if [ $# -eq 4 ] && [ $1 -eq 4 ];
then
IP_MEM=$2
IP_LFS=$3
IP_MEM1=$4
fi

if [ $# -gt 4 ];
then
echo $#
echo "Los parametros habiitados son el numero de memoria y la ip de mem, fs y seed"
exit 1
fi

echo "Script para ejecutar la prueba de kernel en la memoria $1"

cd ..

if [ ! -d Config ];
then
echo "se crea la carpeta de Config"
mkdir Config
fi

cd Config/

if [  -f MEMORIA.txt ];
then
echo "se borra el archivo anterior de config"
rm -r  MEMORIA.txt
fi

cp ../../CONFIGS_SCRIPTS/PRUEBA_KERNEL/MEM$1_CONFIG.txt ./
echo "Config kernel copiado"

mv MEM$1_CONFIG.txt MEMORIA_$1.txt

sed -i -e "s/ip/$IP_MEM/" MEMORIA_$1.txt
sed -i -e "s/fs/$IP_LFS/" MEMORIA_$1.txt

if [ $1 -eq 1 ];
then
sed -i -e "s/mem3/$IP_MEM3/" MEMORIA_$1.txt
fi

if [ $1 -eq 2 ];
then
sed -i -e "s/mem4/$IP_MEM4/" MEMORIA_$1.txt
fi

if [ $1 -eq 4 ];
then
sed -i -e "s/mem1/$IP_MEM1/" MEMORIA_$1.txt
fi