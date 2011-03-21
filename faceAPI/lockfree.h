//lock free queue template class by Herb Sutter
//Dr Dobbs Journal article http://www.drdobbs.com/cpp/210604448;jsessionid=OQGQPSMNL4X4XQE1GHPSKH4ATMY32JVN?pgno=1

template <typename T> class LockFreeQueue
{
private:
	struct Node
	{
		Node( T val ) : value(val), next(nullptr) { }
		T value;
		Node* next;
	};
  
	Node* first;			// for producer only
	Node* divider, last;	// shared

	//not working in VC2008
	//atomic<Node*> divider, last;	// shared

public:
	LockFreeQueue()
	{
		// add dummy separator		
		first = divider = last = new Node( T() );
  	}

	~LockFreeQueue()
	{
		while( first != nullptr )
		{   
			// release the list			
			Node* tmp = first;
			first = tmp->next;
			delete tmp;
		}
	}

	void Produce( const T& t )
	{
		last->next = new Node(t);  	// add the new item
		last = last->next;			// publish it
    
		while( first != divider )
		{	
			// trim unused nodes			
			Node* tmp = first;
			first = first->next;
			delete tmp;
		}
	}

	bool Consume( T& result )
	{
		if( divider != last )
		{
			// if queue is nonempty			
			result = divider->next->value; 	// copy it back
			divider = divider->next; 		// publish that we took it
			return true;          			// and report success
    		}
		
		return false;           			// else report empty
	}
};

