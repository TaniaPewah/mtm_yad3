/*
 * agent.c
 *
 *  Created on: Apr 30, 2016
 *      Author: Tania
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "agent.h"
#include "apartment_service.h"
#include "utilities.h"
#include "map.h"

struct Agent_t {
	Email email;
	char* companyName;
	int taxPercentge;
	Map apartmentServices;
};

static MapDataElement GetDataCopy(constMapDataElement data);
static MapKeyElement GetKeyCopy(constMapKeyElement key);
static void FreeData(MapDataElement data);
static void FreeKey(MapKeyElement key);
static int CompareKeys(constMapKeyElement first, constMapKeyElement second);

static AgentResult squresCreate(int width, int height, char* matrix,
		SquareType*** result);
static void squresDestroy(SquareType** squres, int length);
static AgentResult ConvertServiceResult(ApartmentServiceResult result);
static bool isTaxValid( int taxPercentage );
static bool isPriceValid( int price );

/* isValid: The function checks whether the given apartment numerical
 * 					param is valid
 *
 * @param  The param to check.
 *
 * * @return
 * false if invalid; else returns true.
 */
static bool isValid( int param ){
	return param > 0;
}

 /**
 * agentGetMail: gets the agents mail
 *
 * @param  agent  - Target Agent
 *
 * @return the agent's mail
 */
Email agentGetMail( Agent agent ){
	 return(agent->email);
}

/**
* agentGetCompany: gets the agents companyName
*
* @param  agent  - Target Agent
*
* @return NULL if agent is NULL  else
* 				the agent's companyName
*/
char* agentGetCompany( Agent agent ){
	 return agent ? agent->companyName : NULL;
}

/**
 * agentGetTax: gets the agents tax percentage
 *
 * @param  agent  - Target Agent
 *
 * @return -1 if the agent is NLL else
 * 			the agent's  tax percentage
 */
int agentGetTax( Agent agent ){
	return agent ? agent->taxPercentge : -1;
}

 /**
 * Allocates a new agent.
 *
 * Creates a new agent. This function receives the agent email and agent
 *  company name and tax percentage
 *  and retrieves the new agent created.
 *
 * @param email   		agents email.
 * @param companyName   agents company name
 * @param taxPercentge  percentage of tax the agent takes
 * @param result 		pointer to save the result agent in
 *
 * @return
 *
 * 	AGENT_INVALID_PARAMETERS - if email, companyName or result are NULL or
 * 	taxPercentge is not a valid tax percentage.
 * 	AGENT_OUT_OF_MEMORY - if allocations failed
 * 	AGENT_SUCCESS - in case of success.  A new agent is saved in the result
 * 	parameter.
 */
 AgentResult agentCreate(Email email, char* companyName, int taxPercentge,
		 Agent* result) {
 	if ((companyName == NULL) || (result == NULL) || (email == NULL) ||
 			!isTaxValid(taxPercentge))
 		return AGENT_INVALID_PARAMETERS;
 	Agent agent = malloc (sizeof(*agent));
 	if (agent == NULL)
 		return AGENT_OUT_OF_MEMORY;
 	EmailResult eResult = emailCopy( email, &(agent->email));
 	if( eResult == EMAIL_OUT_OF_MEMORY ){
 		free(agent);
 		return AGENT_OUT_OF_MEMORY;
 	}
 	agent->companyName = duplicateString(companyName);
 	if ( agent->companyName == NULL ) {
 		agentDestroy(agent);
 		return AGENT_OUT_OF_MEMORY;
 	}
 	agent->apartmentServices = mapCreate(GetDataCopy, GetKeyCopy,
 			FreeData, FreeKey, CompareKeys);
 	if( agent->apartmentServices == NULL){
 		agentDestroy(agent);
 		return AGENT_OUT_OF_MEMORY;
 	}
	agent->taxPercentge = taxPercentge;
	*result = agent;
	return AGENT_SUCCESS;
 }

 /**
 * AgentDestroy: Deallocates an existing agent.
 * Clears the element by using the stored free function.
 *
 * @param agent Target agent to be deallocated.
 * If agent is NULL nothing will be done
 */
 void agentDestroy(Agent agent) {
 	if (agent != NULL) {
 		emailDestroy( agent->email );
 		free(agent->companyName);
 		if(agent->apartmentServices != NULL){
 			mapDestroy( agent->apartmentServices );
 			agent->apartmentServices = NULL;
 		}
 		free(agent);
 	}
 }

 /**
 * agentGetService: gets the apartment service according to name
 *
 * @param agent - target agent
 * @param serviceName  the name of the requested service
 *
 * @return
 *
 * 	NULL  if agent is NULL or the service by this name is not found
 *	apartment service otherwise
 */
ApartmentService agentGetService(Agent agent, char* serviceName) {
	ApartmentService service = NULL;
	if(serviceName != NULL)
		service = mapGet(agent->apartmentServices, serviceName);
	return service;
}

/**
 * agentAddService: adds the apartment service to the requested agent
 *
 * @param agent - target agent
 * @param serviceName  the name of the requested service
 * @param max_apartments the max number of apartments allowed in service
 *
 * @return
 *
 * 	AGENT_INVALID_PARAMETERS  if any of the parameters is NULL
 * 	AGENT_OUT_OF_MEMORY       if failed to add the service to serviceMap
 *	AGENT_SUCCESS    		  if succeeded
 */
AgentResult agentAddService(Agent agent, char* serviceName,
		int max_apartments) {
	if((agent == NULL) || (serviceName == NULL) || (max_apartments <= 0) ||
		(max_apartments > 100)) return AGENT_INVALID_PARAMETERS;
	ApartmentService service = serviceCreate(max_apartments);
	if (service == NULL) return AGENT_OUT_OF_MEMORY;
	MapResult result = mapPut(agent->apartmentServices,
			(constMapKeyElement)serviceName, (constMapDataElement)service);
	serviceDestroy(service);
	if (result != MAP_SUCCESS) return AGENT_OUT_OF_MEMORY;
	return AGENT_SUCCESS;
}

/**
 * agentAddService: removes the apartment service from the requested agent
 *
 * @param agent - target agent
 * @param serviceName  the name of the requested service
 *
 * @return
 *
 * 	AGENT_INVALID_PARAMETERS  			if any of the parameters is NULL
 * 	AGENT_APARTMENT_SERVICE_NOT_EXISTS 	if the service does not exist
 *	AGENT_SUCCESS    		  			if succeeded
 */
AgentResult agentRemoveService( Agent agent, char* service_name ){
	if((agent == NULL) || (service_name == NULL))
		return AGENT_INVALID_PARAMETERS;
	MapResult result =
		mapRemove( agent->apartmentServices, (constMapKeyElement)service_name);
	if ( result == MAP_NULL_ARGUMENT )
		return AGENT_INVALID_PARAMETERS;
	if ( result == MAP_ITEM_DOES_NOT_EXIST )
		return AGENT_APARTMENT_SERVICE_NOT_EXISTS;
	return AGENT_SUCCESS;
}

/**
* agentAddApartmentToService: add apartment to apartment service
* 									  of requested agent
* @param agent   	 the requested agent
* @param serviceName a name of the service to add the apartment to
* @param apartment	 the apartment to add
*
* @return
*	AGENT_INVALID_PARAMETERS   if any of parameters are NULL
*	AGENT_APARTMENT_SERVICE_FULL       if service is full
*	AGENT_APARTMENT_SERVICE_NOT_EXISTS service with the given name doesn't exist
*	AGENT_APARTMENT_EXISTS     if apartment with the given id already exist
*	AGENT_OUT_OF_MEMORY      if allocation failed
*	AGENT_SUCCESS            if apartment successfully added
*/
AgentResult agentAddApartmentToService(Agent agent, char* service_name, int id,
		int price, int width, int height, char* matrix) {
	if ((agent == NULL) || (service_name == NULL) ||
		(id < 0) || (price <= 0) || !isPriceValid(price) ||
		(width <= 0) || (height <= 0) || (matrix == NULL) ||
		(strlen(matrix) != (width * height))) return AGENT_INVALID_PARAMETERS;
	ApartmentService service = agentGetService(agent, service_name);
	if (service == NULL) return AGENT_APARTMENT_SERVICE_NOT_EXISTS;
	SquareType** squares = NULL;
	AgentResult squre_result = squresCreate(width, height, matrix, &squares);
	if (squre_result != AGENT_SUCCESS) return squre_result;
	Apartment apartment = apartmentCreate(squares, height, width, price);
	if (apartment == NULL) {
		squresDestroy(squares, height);
		return AGENT_OUT_OF_MEMORY;
	}
	ApartmentServiceResult result = serviceAddApartment(service, apartment, id);
	apartmentDestroy(apartment);
	squresDestroy(squares, height);
	return ConvertServiceResult(result);
}

/**
* squresCreate: creates a SquareType metrix in the given width and hight,
* 	according to a given string built from 'e's and 'w's, e stands for empty
* 	and w stands from wall
*
* @param width   	the matrix width
* @param height  	the matrix height
* @param matrix		matrix string
* @param result 	pointer to result
*
* @return
*	AGENT_INVALID_PARAMETERS   if any of parameters are NULL
*	AGENT_APARTMENT_NOT_EXISTS if apartment not found in the service
* 	AGENT_APARTMENT_SERVICE_NOT_EXISTS service with the given name doesn't exist
*	AGENT_OUT_OF_MEMORY      if allocation failed
*	AGENT_SUCCESS            if apartment successfully added
*/
static AgentResult squresCreate(int width, int height, char* matrix,
	SquareType*** result) {
	SquareType** squre = malloc (sizeof(*squre) * height);
	if (squre == NULL) return AGENT_OUT_OF_MEMORY;
	for (int row = 0; row < height; row++) {
		squre[row] = NULL;
	}
	for (int row = 0; row < height; row++) {
		squre[row] = malloc (sizeof(squre) * width);
		if (squre[row] == NULL) {
			squresDestroy(squre, height);
			return AGENT_OUT_OF_MEMORY;
		} else {
			for (int col = 0; col < width; col++) {
				if (matrix[(row * width) + col] == WALL_CHAR) {
					squre[row][col] = WALL;
				} else if (matrix[(row * width) + col] == EMPTY_CHAR) {
					squre[row][col] = EMPTY;
				} else {
					squresDestroy(squre, height);
					return AGENT_INVALID_PARAMETERS;
				}
			}
		}
	}
	*result = squre;
	return AGENT_SUCCESS;
}

/*
 * Destroys the given squre
 */
static void squresDestroy(SquareType** squres, int length) {
	if (squres != NULL) {
		for (int i = 0; i < length; i++) {
			free(squres[i]);
		}
		free(squres);
	}
}

/*
 * ConvertServiceResult ApartmentServiceResult to AgentResult
 */
static AgentResult ConvertServiceResult(ApartmentServiceResult value) {
	AgentResult result;
	switch (value){
		case APARTMENT_SERVICE_OUT_OF_MEM : {
			result = AGENT_OUT_OF_MEMORY;
			break;
		}
		case APARTMENT_SERVICE_ALREADY_EXISTS:{
			result = AGENT_APARTMENT_EXISTS;
			break;
		}
		case APARTMENT_SERVICE_FULL:{
			result = AGENT_APARTMENT_SERVICE_FULL;
			break;
		}
		case APARTMENT_SERVICE_NULL_ARG: {
			result = AGENT_INVALID_PARAMETERS;
			break;
		}
		case APARTMENT_SERVICE_EMPTY:
		case APARTMENT_SERVICE_NO_FIT:
		{
			result = AGENT_APARTMENT_NOT_EXISTS;
			break;
		}
		case APARTMENT_SUCCESS:
		default: {
			result = AGENT_SUCCESS;
			break;
		}
	}
	return result;
}

/**
* agentRemoveApartmentFromService: remove apartment from apartment service
* 									  of requested agent
* @param agent   	 the requested agent
* @param apartmentId  the apartmentId to remove
* @param serviceName a name of the service to remove the apartment from
*
* @return
*	AGENT_INVALID_PARAMETERS   if any of parameters are NULL
*	AGENT_APARTMENT_NOT_EXISTS if apartment not found in the service
* 	AGENT_APARTMENT_SERVICE_NOT_EXISTS service with the given name doesn't exist
*	AGENT_OUT_OF_MEMORY      if allocation failed
*	AGENT_SUCCESS            if apartment successfully added
*/
AgentResult agentRemoveApartmentFromService( Agent agent, int apartmentId,
											char* serviceName ){
	if(agent == NULL || !isValid( apartmentId) || serviceName == NULL)
		return AGENT_INVALID_PARAMETERS;
	ApartmentService service = agentGetService(agent, serviceName);
	if (service == NULL)
		return AGENT_APARTMENT_SERVICE_NOT_EXISTS;
	Apartment apartment = NULL;
	ApartmentServiceResult getResult = serviceGetById(service,
			apartmentId, &apartment);
	if( getResult == APARTMENT_SERVICE_NULL_ARG ) {
		apartmentDestroy(apartment);
		return AGENT_INVALID_PARAMETERS;
	}
	if( getResult != APARTMENT_SERVICE_SUCCESS ) {
		apartmentDestroy(apartment);
		return AGENT_APARTMENT_NOT_EXISTS;
	}
	ApartmentServiceResult deleteResult = serviceDeleteApartment(service,
		apartment);
	apartmentDestroy(apartment);
	return ConvertServiceResult(deleteResult);
}

static AgentResult findMatch(ApartmentService service, int rooms, int area,
							int price, Agent agent, AgentDetails* details) {
	ApartmentServiceResult result;
	Apartment apartment;
	result = serviceSearch(service, area, rooms, price, &apartment);
	apartmentDestroy(apartment);
	if(result == APARTMENT_SERVICE_SUCCESS) {
		*details = agentDetailsCreate(agent->email, agent->companyName,
			RANK_EMPTY);
		if(*details == NULL) return AGENT_OUT_OF_MEMORY;
		return AGENT_SUCCESS;
	}
	return AGENT_APARTMENT_NOT_EXISTS;
}

/**
* agentFindMatch: finds a matching apartment in each of the agent's services
*
* @param agent   	the requested agent
* @param rooms  the minimum amounts of rooms in the requested apartment
* @param area   the minimum area in the requested apartment
* @param price  the maximum price of the apartment
* @param details out parameter AgentDetails instance with the details of the
* 				 agent - if the requested apartment was found in at least one
* 				 of the apartment services that the
*
* @return
*   AGENT_INVALID_PARAMETERS			if any of the parameters are NULL
*	AGENT_APARTMENT_NOT_EXISTS          if the matching apartment is not found
*	AGENT_APARTMENT_SERVICE_NOT_EXISTS  if the current agent has no apartment
*										services
*	AGENT_OUT_OF_MEMORY                 if any of the allocations failed
*	AGENT_SUCCESS                       a match is found
*/
AgentResult agentFindMatch(Agent agent, int rooms, int area,
							int price, AgentDetails* details) {
	if(agent == NULL || details == NULL) return AGENT_INVALID_PARAMETERS;
	AgentResult result;
	char* name = mapGetFirst(agent->apartmentServices);
	if (name == NULL) return AGENT_APARTMENT_SERVICE_NOT_EXISTS;
	ApartmentService curr_service = mapGet(agent->apartmentServices, name);
	if(curr_service == NULL) return AGENT_APARTMENT_SERVICE_NOT_EXISTS;
	while(curr_service != NULL) {
		result = findMatch(curr_service, rooms, area, price, agent, details);
		name = mapGetNext(agent->apartmentServices);
		 curr_service = (name != NULL) ?
				 mapGet(agent->apartmentServices, name) : NULL;
	}
	return result;
}

/**
* agentGetRank: calculates the rank of the agent according to a formula
*
* @param agent 	the requested agent
*
* @return
*	the rank of the agent agent if agent has at least 1 apartment, and
*	RANK_EMPTY if has no apartments
*/
double agentGetRank(Agent agent) {
	if( agent == NULL ) return AGENT_INVALID_PARAMETERS;
	int apartments_count = 0, median_price = 0, median_area = 0,
		area = 0, price;
	char* name = mapGetFirst(agent->apartmentServices);
	ApartmentService current = NULL;
	if(name != NULL) current = mapGet(agent->apartmentServices, name);
	while(current != NULL) {
		if ((serviceAreaMedian(current, &area) == APARTMENT_SERVICE_SUCCESS) &&
			(servicePriceMedian(current, &price) == APARTMENT_SERVICE_SUCCESS))
		{
			median_area += area;
			median_price += price;
			apartments_count++;
		}
		name = mapGetNext(agent->apartmentServices);
		current = ((name != NULL) ?
					mapGet(agent->apartmentServices, name) :
					NULL);
	}
	if (apartments_count != 0) {
		median_price /= apartments_count;
		median_area /= apartments_count;
	}
	return apartments_count ?
		(1000000 * apartments_count + median_price + 100000 * median_area)
		: RANK_EMPTY;
}

/* isTaxValid: The function checks whether the tax is between 1 and 100
 *
 * @taxPercentage  The tax to check.
 *
 * @return
 * false if invalid; else returns true.
 */
static bool isTaxValid( int taxPercentage ){
	return ((taxPercentage >=1) && (taxPercentage <= 100));
}

/**
* agentCopy: Allocates a new agent, identical to the old agent
*
* Creates a new agent. This function receives a agent, and retrieves
* a new identical agent pointer in the out pointer parameter.
*
* @param agent the original agent.
* @param result pointer to save the new agent in.
*
* @return
*
* 	AGENT_INVALID_PARAMETERS - if agent or pointer -are NULL.
*
* 	AGENT_OUT_OF_MEMORY - if allocations failed.
*
* 	AGENT_SUCCESS - in case of success. A new agent is saved in the result.
*/
AgentResult agentCopy(Agent agent, Agent* result_agent){
	if (agent == NULL || result_agent == NULL) return AGENT_INVALID_PARAMETERS;
	Agent copy_agent = NULL;
	AgentResult result_state = agentCreate( agent->email,
				agent->companyName, agent->taxPercentge, &copy_agent);
	if (result_state != AGENT_SUCCESS) return  AGENT_OUT_OF_MEMORY;

	mapDestroy(copy_agent->apartmentServices);
	copy_agent->apartmentServices = mapCopy(agent->apartmentServices);
 	if( copy_agent->apartmentServices == NULL){
 		agentDestroy(copy_agent);
 		return AGENT_OUT_OF_MEMORY;
 	}
 	*result_agent = copy_agent;
	return AGENT_SUCCESS;
}

/**
* agentGetApartmentDetails: finds the apartment and retrieves its details
*
* @param agent			 the agent
* @param service_name	 the apartment's service name
* @param id				 the apartment's id
* @param apartment_area	 pointer to save the apartment's area
* @param apartment_rooms pointer to save the apartment's room count
* @param apartment_price pointer to save the apartment's price
*
* @return
* 	AGENT_INVALID_PARAMETERS if agent, service_name,
* 		apartment_area, apartment_rooms or apartment_price are NULL.
*
* 	AGENT_APARTMENT_SERVICE_NOT_EXISTS if agent has no service under the
* 		given service name.
*
*	AGENT_APARTMENT_NOT_EXISTS if the matching apartment is not found
*
*	AGENT_SUCCESS apartment found.
*/
AgentResult agentGetApartmentDetails(Agent agent, char* service_name,
	int id, int *apartment_area, int *apartment_rooms, int *apartment_price) {
	if ((agent == NULL) || (service_name == NULL) || (apartment_area == NULL)
		|| (apartment_rooms == NULL) ||	(apartment_price == NULL) ||
		!isValid(id) ) return AGENT_INVALID_PARAMETERS;
	ApartmentService service = mapGet(agent->apartmentServices, service_name);
	if (service == NULL) return AGENT_APARTMENT_SERVICE_NOT_EXISTS;
	Apartment apartment = NULL;
	ApartmentServiceResult result = serviceGetById(service, id, &apartment);
	if (result != APARTMENT_SERVICE_SUCCESS) {
		if (result == APARTMENT_SERVICE_NO_FIT || APARTMENT_SERVICE_EMPTY) {
			return AGENT_APARTMENT_NOT_EXISTS;
		} else return AGENT_OUT_OF_MEMORY;
	}
	*apartment_area = apartmentTotalArea(apartment);
	*apartment_rooms = apartmentNumOfRooms(apartment);
	*apartment_price  = apartmentGetPrice(apartment);
	apartmentDestroy(apartment);
	return AGENT_SUCCESS;
}

/** Function to be used for copying data elements into the map */
static MapDataElement GetDataCopy(constMapDataElement data) {
	ApartmentService new_service = NULL;
	new_service = serviceCopy( (ApartmentService)data );
	return (MapDataElement)new_service;
}

/** Function to be used for copying key elements into the map */
static MapKeyElement GetKeyCopy(constMapKeyElement key) {
	char* name = NULL;
	name = duplicateString( key );
	return name;
}

/** Function to be used for freeing data elements into the map */
static void FreeData(MapDataElement data) {
	if (data != NULL) serviceDestroy((ApartmentService)data);
}

/** Function to be used for freeing key elements into the map */
static void FreeKey(MapKeyElement key) {
	if (key != NULL) free(key);
}

/** Function to be used for comparing key elements in the map */
static int CompareKeys(constMapKeyElement first, constMapKeyElement second) {
	return strcmp( first, second);
}

/* priceisValid: The function checks whether the price can be divided by 100
 *
 * @price  The price to check.
 *
 * * @return
 * false if invalid; else returns true.
 */
static bool isPriceValid( int price ){
	return !(price%100);
}

