#include "parser.h"

char* eliminar_espacios_sobrantes(char* str)
{
	int j=0,i=0;
	char* ret=malloc(strlen(str)+1);
	while(str[i]==' ' && i<strlen(str))
		i++;
	for(;i<strlen(str);i++)
	{
		if(i==strlen(str)-1 && str[i] ==' ')
			continue;
		ret[j]=str[i];
		if(!(str[i]==' ' && str[i+1]==' '))
			j++;
	}
	ret[j]='\0';
	return ret;
}

void borrar_request(request_t req)
{
	for(int i=0; i<req.cant_args; i++){
		free(req.args[i]);
	}
	if(req.cant_args > 0)
		free(req.args);
	free(req.request_str);
	free(req.command_str);
}

int contar_espacios(char* str)
{
	int cont=0, block=0;
	for(int i=0;i<=strlen(str);i++){
		if(str[i]==' '&&!block)
			cont++;
		if(str[i]=='"'){
			if(block)
				block=0;
			else
				block=1;
		}
	}
	return cont;
}

char* primer_palabra(char* str)
{
	char *ret_str=malloc(strlen(str)+1);
	char *aux=malloc(strlen(str)+1);
	int cut;
	strcpy(ret_str,str);
	if(ret_str[0]=='"'){
		strcpy(aux,ret_str);
		aux[0]='-';
		if(aux!=NULL)
			cut=strcspn(aux,"\"")+1;
	}
	else{
		strcpy(aux,ret_str);
		aux[0]='-';
		if(aux!=NULL)
			cut=strcspn(aux," ");		
	}
	if(cut<=strlen(ret_str))
		ret_str[cut]='\0';
	free(aux);
	return ret_str;
}

char* sacar_primeros_caracteres(char* str, int n)
{
	int i;
	char *ret_str;
	if(n<strlen(str)){
		ret_str=malloc(strlen(str)-n+1);
		for(i=0; i<strlen(str)-n;i++){
			ret_str[i]=str[i+n];
		}
		ret_str[i]='\0';
	}
	else{
		ret_str=malloc(strlen(str)+1);
		strcpy(ret_str,str);
	}
	return ret_str;
}

request_t parser(char* req)
{
	char *command,*aux_arg,*args;
	char *temp = eliminar_espacios_sobrantes(req);
	int cant_args=0;
	request_t request;
	request.request_str = malloc(strlen(temp)+1);
	strcpy(request.request_str,temp);
	command=primer_palabra(temp);
	request.command_str=command;
	cant_args=contar_espacios(temp);
//	command=strtok(req," ");
	if(!strcmp(command,"SELECT"))
		request.command=SELECT;
	else if(!strcmp(command,"INSERT"))
		request.command=INSERT;
	else if(!strcmp(command,"CREATE"))
		request.command=CREATE;
	else if(!strcmp(command,"DESCRIBE"))
		request.command=DESCRIBE;
	else if(!strcmp(command,"DROP"))
		request.command=DROP;
	else if(!strcmp(command,"JOURNAL"))
		request.command=JOURNALCOMANDO;
	else if(!strcmp(command,"SALIR")) {
			request.command=SALIR;
		}
	else {
		request.command=INVALID_COMMAND;
	}
	args=sacar_primeros_caracteres(temp,strlen(command)+1);
	request.cant_args=cant_args;
	if(cant_args > 0)
	{
		request.args=malloc(sizeof(char*)*cant_args);
		for(int i=0; i<cant_args; i++){
			request.args[i]=primer_palabra(args);
			if(i != cant_args-1){
				aux_arg=sacar_primeros_caracteres(args,strlen(request.args[i])+1);
				free(args);
				args=aux_arg;
			}
		}
	}
	free(args);
	free(temp);
	return request;
}
