template <class VALUE, unsigned int dims, bool copy_data> class BrtArray;

template <class VALUE, unsigned int dims, bool copy_data> class ConstXSpecified {
	unsigned int indices [dims+1];
	const BrtArray<VALUE,dims,copy_data> *const parent;
public:
	ConstXSpecified(const BrtArray<VALUE,dims,copy_data> *const parent,unsigned int i):parent(parent) {
		indices[0]=i;
		indices[dims]=1;
		
	}
	ConstXSpecified<VALUE,dims,copy_data> operator [] (unsigned int i) {
		indices[indices[dims]++]=i;
		return *this;
	}
	operator VALUE () const;
};
template <class VALUE, unsigned int dims, bool copy_data> class XSpecified {
	unsigned int indices [dims+1];
	BrtArray<VALUE,dims,copy_data> *parent;
public:
	XSpecified(BrtArray<VALUE,dims,copy_data> * parent,unsigned int i) {
		indices[0]=i;
		indices[dims]=1;
		this->parent=parent;
	}
	XSpecified<VALUE,dims,copy_data> operator [] (unsigned int i){
		indices[indices[dims]++]=i;
		return *this;
	}
	operator VALUE () const;
	VALUE &operator =  (const VALUE &f)const;
	VALUE &operator += (const VALUE &f)const;
	VALUE &operator *= (const VALUE &f)const;
	VALUE &operator /= (const VALUE &f)const;
	VALUE &operator %= (const VALUE &f)const;
	VALUE &operator -= (const VALUE &f)const;	
};

template <class VALUE, unsigned int dims, bool copy_data> class BrtArray {
	unsigned int extents[dims];
	VALUE * data;
	unsigned int getSize() const{
			unsigned int size=1;
			for (unsigned int i=0;i<dims;++i) {
				size*=extents[i];
			}
			return size;
	}
public:
	BrtArray(VALUE * data, const unsigned int *extents) {
		for (unsigned int i=0;i<dims;++i) {
			this->extents[i]=extents[i];
		}		
		if (!copy_data)
			this->data =data;
		else {
			unsigned int size = getSize();
			this->data = (VALUE *)malloc(sizeof(VALUE)*size);
			memcpy (this->data,data,sizeof(VALUE)*size);
		}

	}
	BrtArray& operator = (const BrtArray<VALUE,dims,copy_data> &c) {
		for (unsigned int i=0;i<dims;++i) {
			extents[i]=c.extents[i];
		}
		this->data = c.data;
		if (copy_data) {
			unsigned int size = getSize();
			this->data = (VALUE *)malloc(sizeof(VALUE)*size);
			memcpy(this->data,c.data,sizeof(VALUE)*size);
		}
		return *this;
	}
	BrtArray(const BrtArray <VALUE,dims,copy_data>&c) {
		*this=c;
	}
	~BrtArray() {
		if (copy_data)
			free(this->data);
	}
	unsigned int linearaddresscalc (const unsigned int * indices)const {
		unsigned int total=0;
		for (unsigned int i=0;i<dims;++i) {
			total*=extents[i];
			total+=indices[i];
		}
		return total;
	}
	VALUE get(const unsigned int * indices)const {
		return data[this->linearaddresscalc(indices)];
	}
	VALUE&get (const unsigned int *indices) {
		return data[this->linearaddresscalc(indices)];
	}
	ConstXSpecified<VALUE,dims,copy_data> operator [] (unsigned int i)const {
		return  ConstXSpecified<VALUE,dims,copy_data>(this,i);
	}
	XSpecified<VALUE,dims,copy_data> operator [] (unsigned int i){
		return  XSpecified<VALUE,dims,copy_data>(this,i);
	}
};

template <class VALUE, unsigned int dims, bool copy_data> XSpecified<VALUE,dims,copy_data>::operator VALUE ()const {
	return parent->get(indices);
}
template <class VALUE, unsigned int dims, bool copy_data> ConstXSpecified<VALUE,dims,copy_data>::operator VALUE ()const {
	return parent->get(indices);
}

#define OPXD(op) template <class VALUE, unsigned int dims, bool copy_data>VALUE &XSpecified<VALUE,dims,copy_data>::operator op (const VALUE &f)const{ \
	VALUE * v = &parent->get(indices); \
	(*v) op f; \
	return *v; \
}
OPXD(=)
OPXD(*=)
OPXD(%=)
OPXD(+=)
OPXD(/=)
OPXD(-=)
#undef OP1D	

