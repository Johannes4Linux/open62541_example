#include "open62541.h"
//#include <signal.h>
//#include <open62541/server.h>
//#include <open62541/server_config_default.h>

#include <signal.h>
#include <stdlib.h>

static volatile UA_Boolean running = true;
static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

UA_Double Temperature = 20.0;

static void
beforeReadTemperature(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeid, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    float tmp = 1.0 * (rand()%100)/100 - 0.5;
	Temperature += tmp;

	UA_Variant value;
    UA_Variant_setScalar(&value, &Temperature, &UA_TYPES[UA_TYPES_DOUBLE]);		//Copy the Temperature in a variant variable
    UA_Server_writeValue(server, UA_NODEID_STRING(2, "R1_TS1_Temperature"), value);

}

static void
afterWriteState(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    UA_Variant value;
	UA_Boolean state;

	UA_Server_readValue(server, UA_NODEID_STRING(2, "R1_LB1_State"), &value); 
	state = *(UA_Boolean*) value.data;

	if(state == true)
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "The Light Bulb is now ON");
	else
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "The Light Bulb is now OFF");

}

int main(int argc, char * argv[]) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_Server *server = UA_Server_new();

	//Check for arguments
	if(argc > 2)	//hostname or ip address and a port number are available
	{				
		UA_Int16 port_number = atoi(argv[2]);
		UA_ServerConfig_setMinimal(UA_Server_getConfig(server), port_number, 0);
	}
	else
    	UA_ServerConfig_setDefault(UA_Server_getConfig(server));
	
	if(argc > 1)
	{				//hostname or ip address available
		//copy the hostname from char * to an open62541 variable
		UA_String hostname;
		UA_String_init(&hostname);
		hostname.length = strlen(argv[1]);
		hostname.data = (UA_Byte *) argv[1];

		//Change the configuration
		UA_ServerConfig_setCustomHostname(UA_Server_getConfig(server), hostname);
	}

	//Add a new namespace to the server
	UA_Int16 ns_room1 = UA_Server_addNamespace(server, "Room1");
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "New Namespace added with Nr. %d", ns_room1);

	//Add a new object called Temperature Sensor
    UA_NodeId r1_tempsens_Id; /* get the nodeid assigned by the server */
    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    UA_Server_addObjectNode(server, UA_NODEID_STRING(2, "R1_TemperatureSensor"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(2, "Temperature Sensor"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            oAttr, NULL, &r1_tempsens_Id);
	
	//Add the variable Vendor Name to server
	UA_VariableAttributes vnAttr = UA_VariableAttributes_default;
    UA_String vendorName = UA_STRING("Sensor King Ltd.");
    UA_Variant_setScalar(&vnAttr.value, &vendorName, &UA_TYPES[UA_TYPES_STRING]);
    UA_Server_addVariableNode(server, UA_NODEID_STRING(2, "R1_TS1_VendorName"), r1_tempsens_Id,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(2, "VendorName"),
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), vnAttr, NULL, NULL);


	//Add the variable Serial Number to server
	UA_VariableAttributes snAttr = UA_VariableAttributes_default;
	UA_Int32 serialNumber = 12345678;
    UA_Variant_setScalar(&snAttr.value, &serialNumber, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_addVariableNode(server, UA_NODEID_STRING(2, "R1_TS1_SerialNumber"), r1_tempsens_Id,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(2, "SerialNumber"),
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), snAttr, NULL, NULL);
	

	//Add the variable Temperature to server
	UA_VariableAttributes tpAttr = UA_VariableAttributes_default;
    UA_Variant_setScalar(&tpAttr.value, &Temperature, &UA_TYPES[UA_TYPES_DOUBLE]);
    UA_Server_addVariableNode(server, UA_NODEID_STRING(2, "R1_TS1_Temperature"), r1_tempsens_Id,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(2, "Temperature"),
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), tpAttr, NULL, NULL);

	// Add Callback to Temperature Node
    UA_ValueCallback callback ;
    callback.onRead = beforeReadTemperature;
    callback.onWrite = NULL;
    UA_Server_setVariableNode_valueCallback(server, UA_NODEID_STRING(2, "R1_TS1_Temperature"), callback);
	
	//Add a new object called Light Bulb
    UA_NodeId r1_lightbulb_Id; /* get the nodeid assigned by the server */
    UA_ObjectAttributes o1Attr = UA_ObjectAttributes_default;
    UA_Server_addObjectNode(server, UA_NODEID_STRING(2, "R1_LightBulb"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(2, "Light Bulb"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            o1Attr, NULL, &r1_lightbulb_Id);
	
	//Add the variable Vendor Name to server
	UA_VariableAttributes vn1Attr = UA_VariableAttributes_default;
    UA_String vendorName1 = UA_STRING("BrightLight");
    UA_Variant_setScalar(&vn1Attr.value, &vendorName1, &UA_TYPES[UA_TYPES_STRING]);
    UA_Server_addVariableNode(server, UA_NODEID_STRING(2, "R1_LB1_VendorName"), r1_lightbulb_Id,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(2, "Vendor Name"),
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), vn1Attr, NULL, NULL);


	//Add the variable Serial Number to server
	UA_VariableAttributes sn1Attr = UA_VariableAttributes_default;
	UA_Int32 serialNumber1 = 986543214;
    UA_Variant_setScalar(&sn1Attr.value, &serialNumber1, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_addVariableNode(server, UA_NODEID_STRING(2, "R1_LB1_SerialNumber"), r1_lightbulb_Id,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(2, "SerialNumber"),
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), sn1Attr, NULL, NULL);
	
	//Add the variable state to the lightbulb
	UA_VariableAttributes stateAttr = UA_VariableAttributes_default;
	UA_Boolean state = false;
    UA_Variant_setScalar(&stateAttr.value, &state, &UA_TYPES[UA_TYPES_BOOLEAN]);
	stateAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_Server_addVariableNode(server, UA_NODEID_STRING(2, "R1_LB1_State"), r1_lightbulb_Id,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(2, "State of light bulb"),
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), stateAttr, NULL, NULL);
	
	//Add Callback to state of light bulb	
    UA_ValueCallback callback1;
    callback1.onRead = NULL;
    callback1.onWrite = afterWriteState;
    UA_Server_setVariableNode_valueCallback(server, UA_NODEID_STRING(2, "R1_LB1_State"), callback1);

    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "starting server...");
    UA_StatusCode retval = UA_Server_run(server, &running);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Server was shut down");

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}

