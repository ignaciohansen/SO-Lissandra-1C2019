#!/bin/bash

if [ $# = 0 ];
then
echo "Estaba esperando 1 parametro para el numero de memoria y un opcional para el ip"
exit 1
fi

IP_MEM=127.0.0.1
IP_LFS=127.0.0.1
IP_MEM2=127.0.0.1
IP_MEM3=127.0.0.1

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
IP_MEM2=$4
fi

if [ $# -eq 4 ] && [ $1 -eq 2 ];
then
IP_MEM=$2
IP_LFS=$3
IP_MEM3=$4
fi

if [ $# -eq 4 ] && [ $1 -eq 3 ];
then
echo "La memoria 3 no tiene seeds"
exit 1
fi

if [ $# -gt 4 ];
then
echo $#
echo "Los parametros habiitados son el numero de memoria y la ip de mem, fs y seed"
exit 1
fi

echo "Script para ejecutar la prueba de LFS en el FileSystem"

cd ..

if [ ! -d Config ];
then
echo "se crea la carpeta de Config"
mkdir Config
fi

cd Config/

if [  -f MEM$1_CONFIG.txt ];
then
echo "se borra el archivo anterior de config"
rm -r  MEM$1_CONFIG.txt
fi

cp ../../CONFIGS_SCRIPTS/PRUEBA_LFS/MEM$1_CONFIG.txt ./
echo "Config lfs copiado"

mv MEM$1_CONFIG.txt MEMORIA_$1.txt

sed -i -e "s/ip/$IP_MEM/" MEMORIA_$1.txt
sed -i -e "s/fs/$IP_LFS/" MEMORIA_$1.txt

if [ $1 -eq 1 ];
then
sed -i -e "s/mem2/$IP_MEM2/" MEMORIA_$1.txt
echo "se cambio"
fi

if [ $1 -eq 2 ];
then
sed -i -e "s/mem3/$IP_MEM3/" MEMORIA_$1.txt
fi