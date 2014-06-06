#include <assert.h>
#include <stdio.h>
#include "ssd.h"

using namespace ssd;


Cmdq::Cmdq()
{
	LOG();

	head = NULL;
	tail = NULL;
	size = 0;

	return;
}

Cmdq::~Cmdq(void)
{
	LOG();
	return;
}

void Cmdq::enqueue(Event &evt)
{
	LOG();

	if(head == NULL)
	{
		head = &evt;
	}else{
		tail->set_next(evt);
	}
	tail = &evt;
	size++;
}

Event *Cmdq::dequeue(void)
{
	LOG();
	Event *e = head;
	head = e->get_next();

	printf("head->get_logical_address() : %lu\n", head->get_logical_address());
	printf("e->get_logical_address() : %lu\n", e->get_logical_address());

	size--;
	return e;
}

void Cmdq::show(void)
{
	LOG();
	printf("----------------------\n");
	for(Event *e = head;e != NULL;e=e->get_next())
	{
//		e->print();
		if(e->get_event_type() == 0) printf("READ  ");
		else 						printf("WRITE ");
		printf("%lu, ", e->get_logical_address());
		printf("%u, ", e->get_size());
		printf("%lf\n", e->get_start_time());
	}
	printf("----------------------\n");
}
