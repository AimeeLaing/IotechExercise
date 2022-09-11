#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

struct devices{
    char * name, * type, * info;
    int payloadTotal;
};

struct devices * parseJson(FILE * f) {
    char * data;
    json_object * jobj;
    json_object * devices;
    json_object * arrayObj;
    json_object * sensors;
    json_object * sensorsArray;
    int numDevices;
    fseek(f,0L,SEEK_END);
    long size = ftell(f);
    fseek(f, 0L, SEEK_SET);
    data = malloc(size +1L);
    fread(data,1,size+1L,f);

    jobj = json_tokener_parse(data);
    devices = json_object_object_get(jobj, "Devices");
    numDevices = json_object_array_length(devices);
    struct devices * deviceInfo = malloc(sizeof(*deviceInfo) * (numDevices));;
    for(int i = 0; i < numDevices;i++){
        arrayObj = json_object_array_get_idx(devices, i);
        deviceInfo[i].name = json_object_get_string(json_object_object_get(arrayObj, "Name"));
        deviceInfo[i].type = json_object_get_string(json_object_object_get(arrayObj, "Type"));
        deviceInfo[i].info = json_object_get_string(json_object_object_get(arrayObj, "Info"));
        sensors = json_object_object_get(arrayObj, "Sensors");
        sensorsArray = json_object_array_get_idx(sensors, 0);
        deviceInfo[i].payloadTotal = json_object_get_int(json_object_object_get(sensorsArray, "Payload"));
        sensorsArray = json_object_array_get_idx(sensors, 1);
        deviceInfo[i].payloadTotal += json_object_get_int(json_object_object_get(sensorsArray, "Payload"));

    }
    return deviceInfo;
}
void outputJson(struct devices * deviceInfo){
    json_object * jobj = json_object_new_object();
    json_object * devices = json_object_new_array();
    char * info;
    char * uuid;
    char * uuid2;
    int location;
    FILE * output;
    for(int i = 0; i < 5; i++){
        json_object * curDevice = json_object_new_object();
        json_object_object_add(curDevice,"Name",json_object_new_string(deviceInfo[i].name));
        json_object_object_add(curDevice,"Type",json_object_new_string(deviceInfo[i].type));
        uuid = strstr(deviceInfo[i].info, "uuid");
        info = deviceInfo[i].info;
        location = uuid - deviceInfo[i].info;
        strncpy(uuid2, deviceInfo[i].info + (location + 5),36);
        for(int i = location -1; i < (location + 41); i ++ ){
            if(info[i + 41] != NULL){
                info[i] = info[i + 42];
            }
            else{
                info[i] = "";
            }
        }
        json_object_object_add(curDevice,"Info",json_object_new_string(info));
        json_object_object_add(curDevice,"Uuid",json_object_new_string(uuid2));
        json_object_object_add(curDevice,"PayloadTotal",json_object_new_int(deviceInfo[i].payloadTotal));
        json_object_array_add(devices, curDevice);
    }
    json_object_object_add(jobj,"Devices", devices);
    output = fopen("output.json", "w");
    fprintf(output, "%s", json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
    fclose(output);
}

int main(int argc,char ** argv){   
    FILE * file = fopen(argv[1], "r");
    if(file ==NULL){  
        printf("Can't open file %s\n",argv[1]);
      	exit(0);
   	}

    struct devices * deviceInfo = parseJson(file);


    outputJson(deviceInfo);
    fclose(file);
}