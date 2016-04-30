/*
 * Client.h
 *
 *  Created on: 30 ����� 2016
 *      Author: binder
 */

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

#define NO_CLIENT_VAL 0
#define AT_SIGN '@'

typedef struct Cliet_t *Client;

/**
* Allocates a new client.
*
* Creates a new client. This function receives the client email, apartment
* preferences and retrieves the new client created.
*
* @param email clients email.
* @param apartment_min_area minimal area for the clients wanted apartments
* @param apartment_min_rooms minimal room count in clients wanted apartments
* @param apartment_max_price maximum price for the clients wanted apartments
*
* @return
* 	NULL - if one email is NULL or does not contain the character AT_SIGN,
* 	apartment_min_area apartment_min_rooms or apartment_max_price is not bigger
* 	then zero  or allocations failed.
* 	A new clients in case of success.
*/
Client clientCreate(const char* email, int apartment_min_area,
		int apartment_min_rooms, int apartment_max_price);

/**
* ClientDestroy: Deallocates an existing client.
* Clears all elements by using the stored free function.
*
* @param client Target client to be deallocated.
* If client is NULL nothing will be done
*/
void clientDestroy(Client client);

/**
* clientGetMail: gets the given client email.
*
* @param client Target client.
*
* @return
* 	NULL - if client is NULL
* 	The clients email in case of success.
*/
char* clientGetMail(Client client);

/**
* clientGetMinArea: gets the given client minimal apartment area wanted.
*
* @param client Target client.
*
* @return
* 	NO_CLIENT_VAL - if client is NULL
* 	The clients minimal apartment area wanted in case of success.
*/
int clientGetMinArea(Client client);

/**
* clientGetMinRooms: gets the given client minimal room count wanted.
*
* @param client Target client.
*
* @return
* 	NO_CLIENT_VAL - if client is NULL
* 	The clients minimal room count wanted in case of success.
*/
int clientGetMinRooms(Client client);

/**
* clientGetMaxPrice: gets the given client maximum price that can be paid for
* an apartment.
*
* @param client Target client.
*
* @return
* 	NO_CLIENT_VAL - if client is NULL
* 	The clients  maximum price that can be paid for an apartment.
*/
int clientGetMaxPrice(Client client);

/**
* clientAddPaiment: Adds an apartment payment to the given client.
*
* @param client Target client.
* @param payment payment to add.
*
* If client is NULL or payment is negative nothing will be done
*/
void clientAddPayment(Client client, int payment);

/**
* clientGetTotalPayments: gets the total payments sum the client payed.
* @param client Target client.
*
* @return
* 	NO_CLIENT_VAL - if client is NULL
* 	or the clients total payments sum in case of success.
*/
int clientGetTotalPayments(Client client);


#endif /* SRC_CLIENT_H_ */
