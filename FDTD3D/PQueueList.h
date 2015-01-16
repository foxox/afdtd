#ifndef PQUEUELISTH
#define PQUEUELISTH

#include <list>

//#include 

struct MetricIndexTuple
{
	float metric;
	unsigned int i;
	unsigned int j;
	unsigned int k;
};

struct PQListNode
{
	PQListNode* next;
	PQListNode* prev;
	MetricIndexTuple mit;
	size_t timestamp;

	PQListNode(MetricIndexTuple _mit)
	{
		this->mit = _mit;
		this->next = NULL;
		this->prev = NULL;
		this->timestamp = 0;
	}

	virtual ~PQListNode()
	{}
};

//Class represents a sorted list of 3d array cells based on some metric
//List is not guaranteed to store any cells more than updateZoneSize, but it may
class PQList
{
private:
public:
	PQListNode* head;
	PQListNode* bottomOfUpdateZone;	//always updateZoneSize down
	size_t updateZoneSize;
	size_t size;

	PQList()
	{
		this->head = NULL;
		this->bottomOfUpdateZone = NULL;
		updateZoneSize = 0;
		size = 0;
	}
	virtual ~PQList()
	{
		if (this->head != NULL)
		{
			PQListNode* node = head;
			while (node->next != NULL)
			{
				node = node->next;
				delete node->prev;
			}
			delete node;
		}
	}

	inline void findBottomOfUpdateZone()
	{
		PQListNode* node = this->head;
		if (node == NULL) return;
		if (this->updateZoneSize == 1)
		{
			this->bottomOfUpdateZone = this->head;
			return;
		}
		size_t i = 0;
		for (i = 1; i < this->updateZoneSize; i++)
		{
			if (node->next == NULL)
				return;
			node = node->next;
			if (i == this->updateZoneSize-1)
				this->bottomOfUpdateZone = node;
		}
	}

	inline void insertBelowUpdateZone(MetricIndexTuple _mit)
	{
		PQListNode* insertme = new PQListNode(_mit);
		this->insertBelowUpdateZone(insertme);
		//insertme->next = this->bottomOfUpdateZone->next;
		//insertme->prev = this->bottomOfUpdateZone;
		//this->bottomOfUpdateZone->next->prev = insertme;
		//this->bottomOfUpdateZone->next = insertme;
	}
	
	inline void insertBelowUpdateZone(PQListNode* insertme)
	{
		if (this->bottomOfUpdateZone == NULL)
		{
			this->push_back(insertme);
			this->findBottomOfUpdateZone();
		}
		else
		{
			insertme->next = this->bottomOfUpdateZone->next;
			insertme->prev = this->bottomOfUpdateZone;
			this->bottomOfUpdateZone->next->prev = insertme;
			this->bottomOfUpdateZone->next = insertme;
		}
		this->size++;
	}

	inline void push_front(MetricIndexTuple _mit)
	{
		PQListNode* insertme = new PQListNode(_mit);
		this->push_front(insertme);
	}

	inline void push_front(PQListNode* insertme)
	{
		if (this->head == NULL)
		{
			this->head = insertme;
		}
		else
		{
			insertme->next = this->head;
			this->head->prev = insertme;
			this->head = insertme;
			insertme->prev = NULL;
		}
		this->size++;
	}

	inline void push_back(PQListNode* insertme)
	{
		if (this->head == NULL)
		{
			this->head = insertme;
		}
		else
		{
			//find end
			PQListNode* node = head;
			if (this->bottomOfUpdateZone != NULL) node = this->bottomOfUpdateZone;
			while (node->next != NULL) node = node->next;
			//insert
			node->next = insertme;
			insertme->next = NULL;
			insertme->prev = node;
		}
		this->size++;
	}

	static inline void sortNode(PQListNode* node)
	{
		while(true)
		{
			if (node->prev != NULL && node->mit.metric > node->prev->mit.metric)
			{
				PQListNode* tempnn = node->next;
				PQListNode* tempnp = node->prev;
				PQListNode* tempnpp = node->prev->prev;
				tempnpp->next = node;
				node->prev = tempnpp;
				node->next = tempnp;
				tempnp->prev = node;
				tempnp->next = tempnn;
				tempnn->prev = tempnp;
			}
			else if (node->next != NULL && node->mit.metric < node->next->mit.metric)
			{
				PQListNode* tempnn = node->next;
				PQListNode* tempnp = node->prev;
				PQListNode* tempnnn = node->next->next;
				node->next = tempnnn;
				node->prev = tempnn;
				tempnn->next = node;
				tempnn->prev = tempnp;
				tempnp->next = tempnn;
				tempnnn->prev = node;
			}
		}
	}

	inline void pruneBelowUpdateZone()
	{
		PQListNode* node = this->bottomOfUpdateZone;
		if (node == NULL || node->next == NULL)
			return;
		node = node->next;
		this->bottomOfUpdateZone->next = NULL;
		while(node->next != NULL)
		{
			node = node->next;
			delete node->prev;
			this->size--;
		}
		delete node;
		this->size--;
	}
};

#endif