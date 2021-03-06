#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "test_utilities.h"
#include "clientsManager.h"
#include "clientPurchaseBill.h"
#include "list.h"
#include "client.h"

static bool testClientsManagerAddClient();
static bool testClientsManagerRemoveClient();
static bool testClientsManagerCreate();
static bool testClientsManagerClientExists();
static bool testClientsManagerGetSortedPayments();
static bool testClientsManagerExecurePurchase();
static bool testClientsManagerGetRestriction();

int RunClientManagerTest(){

	RUN_TEST(testClientsManagerCreate);
	RUN_TEST(testClientsManagerAddClient);
	RUN_TEST(testClientsManagerRemoveClient);
	RUN_TEST(testClientsManagerClientExists);
	RUN_TEST(testClientsManagerGetSortedPayments);
	RUN_TEST(testClientsManagerExecurePurchase);
	RUN_TEST(testClientsManagerGetRestriction);

	return 0;
}
static bool testClientsManagerCreate(){
	ClientsManager manager = clientsManagerCreate();
	ASSERT_TEST(manager != NULL);
	clientsManagerDestroy(manager);
	return true;
}
static bool testClientsManagerAddClient(){
	Email email = NULL;
	emailCreate("baba@ganosh", &email);
	ClientsManager manager = clientsManagerCreate();
	ASSERT_TEST( clientsManagerAdd( NULL, email, 1, 1, 200) ==
			CLIENT_MANAGER_NULL_PARAMETERS);
	ASSERT_TEST( clientsManagerAdd( manager, NULL, 1, 1, 200) ==
			CLIENT_MANAGER_NULL_PARAMETERS);

	ASSERT_TEST( clientsManagerAdd( manager, email, -2, 1, 200) ==
			CLIENT_MANAGER_INVALID_PARAMETERS);
	ASSERT_TEST( clientsManagerAdd( manager, email, 1, 0, 200) ==
			CLIENT_MANAGER_INVALID_PARAMETERS);
	ASSERT_TEST( clientsManagerAdd( manager, email, 1, 1, -9) ==
			CLIENT_MANAGER_INVALID_PARAMETERS);


	ASSERT_TEST( clientsManagerAdd( manager, email, 1, 1, 200) ==
				CLIENT_MANAGER_SUCCESS);

	ASSERT_TEST( clientsManagerAdd( manager, email, 1, 1, 200) ==
			CLIENT_MANAGER_ALREADY_EXISTS);

	emailDestroy(email);
	clientsManagerDestroy(manager);
	return true;
}

static bool testClientsManagerRemoveClient(){

	Email email = NULL;
	emailCreate("baba@ganosh", &email);
	Email mail = NULL;
	emailCreate("baba@gash", &mail);
	ClientsManager manager = clientsManagerCreate();
	ASSERT_TEST( clientsManagerAdd( manager, email, 1, 1, 200) ==
		CLIENT_MANAGER_SUCCESS);
	ASSERT_TEST( clientsManagerRemove( manager, NULL ) ==
		CLIENT_MANAGER_INVALID_PARAMETERS);
	ASSERT_TEST( clientsManagerRemove( NULL, email ) ==
		CLIENT_MANAGER_INVALID_PARAMETERS);
	ASSERT_TEST( clientsManagerRemove( manager, mail ) ==
		CLIENT_MANAGER_NOT_EXISTS);
	ASSERT_TEST( clientsManagerRemove( manager, email ) ==
		CLIENT_MANAGER_SUCCESS);
	emailDestroy(mail);
	emailDestroy(email);
	clientsManagerDestroy(manager);
	return true;
}

static bool testClientsManagerClientExists(){
	Email email = NULL;
	emailCreate("baba@ganosh", &email);
	Email mail = NULL;
	emailCreate("baba@gash", &mail);
	ClientsManager manager = clientsManagerCreate();
	ASSERT_TEST( clientsManagerAdd( manager, email, 1, 1, 200) ==
				CLIENT_MANAGER_SUCCESS);

	ASSERT_TEST( !clientsManagerClientExists( NULL, email) );
	ASSERT_TEST( !clientsManagerClientExists( manager, NULL));
	ASSERT_TEST( !clientsManagerClientExists( manager, mail));
	ASSERT_TEST( clientsManagerClientExists( manager, email));
	emailDestroy(mail);
	emailDestroy(email);
	clientsManagerDestroy(manager);
	return true;
}
static bool testClientsManagerExecurePurchase(){

	Email email = NULL;
	emailCreate("baba@ganosh", &email);
	Email mail = NULL;
	emailCreate("baba@gash", &mail);
	ClientsManager manager = clientsManagerCreate();
	clientsManagerAdd( manager, email, 1, 1, 200);

	ASSERT_TEST( clientsManagerExecutePurchase( NULL, email, 330) ==
			CLIENT_MANAGER_INVALID_PARAMETERS);
	ASSERT_TEST( clientsManagerExecutePurchase( manager, NULL, 330) ==
			CLIENT_MANAGER_INVALID_PARAMETERS);

	ASSERT_TEST( clientsManagerExecutePurchase( manager, mail, 330) ==
			CLIENT_MANAGER_NOT_EXISTS);

	ASSERT_TEST( clientsManagerExecutePurchase( manager, email, 330) ==
				CLIENT_MANAGER_SUCCESS);

	emailDestroy(mail);
	emailDestroy(email);
	clientsManagerDestroy(manager);
	return true;
}

static bool testClientsManagerGetSortedPayments(){
	Email email = NULL, mail1 = NULL ,mail2 = NULL, mail3 = NULL;
	emailCreate("gaba@ganosh", &email);
	emailCreate("baba@gash", &mail1);
	emailCreate("baba@gash2", &mail2);
	emailCreate("baaa@gash", &mail3);
	ClientsManager manager = clientsManagerCreate();
	clientsManagerAdd( manager, email, 1, 1, 200);
	clientsManagerAdd( manager, mail1, 1, 2, 1000);
	clientsManagerAdd( manager, mail2, 1, 2, 1000);
	clientsManagerAdd( manager, mail3, 1, 2, 1000);

	List clients_list = NULL;
	ASSERT_TEST( clientsManagerGetSortedPayments(NULL, &clients_list) ==
			CLIENT_MANAGER_INVALID_PARAMETERS);
	ASSERT_TEST( clientsManagerGetSortedPayments(manager,NULL) ==
			CLIENT_MANAGER_INVALID_PARAMETERS);
	ASSERT_TEST( clientsManagerGetSortedPayments(manager, &clients_list) ==
				CLIENT_MANAGER_SUCCESS);
	ASSERT_TEST( listGetSize(clients_list) == 0);
	listDestroy(clients_list);

	clientsManagerExecutePurchase( manager, email, 330);
	ASSERT_TEST( clientsManagerGetSortedPayments(manager, &clients_list) ==
				CLIENT_MANAGER_SUCCESS);
	ASSERT_TEST( listGetSize(clients_list) == 1);
	listDestroy(clients_list);

	clientsManagerExecutePurchase( manager, mail1, 900);
	ASSERT_TEST( clientsManagerGetSortedPayments(manager, &clients_list) ==
			CLIENT_MANAGER_SUCCESS);
	ASSERT_TEST( listGetSize(clients_list) == 2);
	ClientPurchaseBill top_bill = listGetFirst(clients_list);
	ASSERT_TEST( emailAreEqual(
			clientPurchaseBillGetClientEmail(top_bill), mail1));
	ClientPurchaseBill second_bill = listGetNext(clients_list);
	ASSERT_TEST( emailAreEqual(
		clientPurchaseBillGetClientEmail(second_bill), email));
	listDestroy(clients_list);

	clientsManagerExecutePurchase( manager, mail2, 900);
	ASSERT_TEST( clientsManagerGetSortedPayments(manager, &clients_list) ==
			CLIENT_MANAGER_SUCCESS);
	ASSERT_TEST( listGetSize(clients_list) == 3);
	top_bill = listGetFirst(clients_list);
	ASSERT_TEST( emailAreEqual(
			clientPurchaseBillGetClientEmail(top_bill), mail1));
	second_bill = listGetNext(clients_list);
	ASSERT_TEST( emailAreEqual(
		clientPurchaseBillGetClientEmail(second_bill), mail2));
	second_bill = listGetNext(clients_list);
	ASSERT_TEST( emailAreEqual(
		clientPurchaseBillGetClientEmail(second_bill), email));
	listDestroy(clients_list);

	clientsManagerExecutePurchase( manager, mail3, 900);
	ASSERT_TEST( clientsManagerGetSortedPayments(manager, &clients_list) ==
			CLIENT_MANAGER_SUCCESS);
	ASSERT_TEST( listGetSize(clients_list) == 4);
	top_bill = listGetFirst(clients_list);
	ASSERT_TEST( emailAreEqual(
			clientPurchaseBillGetClientEmail(top_bill), mail3));
	second_bill = listGetNext(clients_list);
	ASSERT_TEST( emailAreEqual(
		clientPurchaseBillGetClientEmail(second_bill), mail1));
	second_bill = listGetNext(clients_list);
	ASSERT_TEST( emailAreEqual(
		clientPurchaseBillGetClientEmail(second_bill), mail2));
	second_bill = listGetNext(clients_list);
	ASSERT_TEST( emailAreEqual(
		clientPurchaseBillGetClientEmail(second_bill), email));
	listDestroy(clients_list);

	clientsManagerExecutePurchase(manager, email, 900);
	ASSERT_TEST( clientsManagerGetSortedPayments(manager, &clients_list) ==
			CLIENT_MANAGER_SUCCESS);
	ASSERT_TEST( listGetSize(clients_list) == 4);
	top_bill = listGetFirst(clients_list);
	ASSERT_TEST( emailAreEqual(
			clientPurchaseBillGetClientEmail(top_bill), email));
	second_bill = listGetNext(clients_list);
	ASSERT_TEST( emailAreEqual(
		clientPurchaseBillGetClientEmail(second_bill), mail3));
	second_bill = listGetNext(clients_list);
	ASSERT_TEST( emailAreEqual(
		clientPurchaseBillGetClientEmail(second_bill), mail1));
	second_bill = listGetNext(clients_list);
	ASSERT_TEST( emailAreEqual(
		clientPurchaseBillGetClientEmail(second_bill), mail2));
	listDestroy(clients_list);

	emailDestroy(email);
	emailDestroy(mail1);
	emailDestroy(mail2);
	emailDestroy(mail3);
	clientsManagerDestroy(manager);
	return true;
}

static bool testClientsManagerGetRestriction(){

	Email email = NULL;
	emailCreate("gaba@ganosh", &email);
	Email mail = NULL;
	emailCreate("baba@gash", &mail);

	ClientsManager manager = clientsManagerCreate();
	clientsManagerAdd( manager, email, 3, 1, 200);

	int area;
	int rooms_num;
	int price;

	ASSERT_TEST( clientsManagerGetRestriction(NULL, email,
				&area, &rooms_num, &price) == CLIENT_MANAGER_NULL_PARAMETERS);
	ASSERT_TEST( clientsManagerGetRestriction(manager, NULL,
				&area, &rooms_num, &price) == CLIENT_MANAGER_NULL_PARAMETERS);
	ASSERT_TEST( clientsManagerGetRestriction(manager, email,
				NULL, &rooms_num, &price) == CLIENT_MANAGER_NULL_PARAMETERS);
	ASSERT_TEST( clientsManagerGetRestriction(manager, email,
				&area,NULL, &price) == CLIENT_MANAGER_NULL_PARAMETERS);
	ASSERT_TEST( clientsManagerGetRestriction(manager, email,
			&area, &rooms_num, NULL) == CLIENT_MANAGER_NULL_PARAMETERS);

	ASSERT_TEST( clientsManagerGetRestriction(manager, mail,
				&area, &rooms_num, &price) == CLIENT_MANAGER_NOT_EXISTS);

	ASSERT_TEST( clientsManagerGetRestriction(manager, email,
				&area, &rooms_num, &price) == CLIENT_MANAGER_SUCCESS);

	ASSERT_TEST( area == 3);
	ASSERT_TEST( rooms_num == 1);
	ASSERT_TEST( price == 200);

	emailDestroy(email);
	emailDestroy(mail);
	clientsManagerDestroy(manager);
	return true;
}

