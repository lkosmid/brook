#ifndef BRTARRAY_H_
#define BRTARRAY_H_
#ifndef MULTIPLE_ARRAY_BOUNDS_LOOKUPS

template <class VALUE, unsigned int dims, bool copy_data> class __BrtArray {
	unsigned int extents[dims];
	VALUE * data;
	brook::Stream * s;
	unsigned int getSize() const{
			unsigned int size=1;
			for (unsigned int i=0;i<dims;++i) {
				size*=extents[i];
			}
			return size;
	}
	void init(VALUE * data, const unsigned int *extents) {
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

public:
	__BrtArray(VALUE * data, const unsigned int *extents):s(0) {
		init(data,extents);
	}
	__BrtArray(brook::Stream * stm):s(0) {
		init(stm->getData(brook::Stream::READ),stm->getExtents());
		if (copy_data) stm->releaseData(brook::Stream::READ);
		else this->s=stm;
	}
	__BrtArray& operator = (const __BrtArray<VALUE,dims,copy_data> &c) {
		for (unsigned int i=0;i<dims;++i) {
			extents[i]=c.extents[i];
		}
		this->data = c.data;
		if (copy_data) {
			unsigned int size = getSize();
			this->data = (VALUE *)malloc(sizeof(VALUE)*size);
			memcpy(this->data,c.data,sizeof(VALUE)*size);
		}else {
                   brook::Stream * tmpstr=this->s;
                   if (c.s) {
                      this->s=c.s;
                      this->data=this->s->getData(brook::Stream::READ);
                   }
                   if (tmpstr) {
                      tmpstr->releaseData(brook::Stream::READ);
                   }
		}
		return *this;
	}
	__BrtArray(const __BrtArray <VALUE,dims,copy_data>&c):s(0){
		*this=c;
	}
	~__BrtArray() {
		if (copy_data)
			free(this->data);
		if (s)
			s->releaseData(brook::Stream::READ);
	}
	template <class T> unsigned int indexOf (const T &index) const{
           unsigned int i=T::size-1;
           if (dims<T::size)
              i=dims-1;
           float temp = round_float(index.getAt(i));
           unsigned int total=(temp<0)?0:temp>=extents[0]-.4?extents[0]-1:
              (unsigned int)temp;
           for (unsigned int j=1;j<=i;++j) {
              total*=extents[j];
              temp = round_float(index.getAt(i-j));
              total+=(temp<0)?0:temp>=extents[j]-.4?extents[j]-1:
                 (unsigned int)temp;
           }
           return total;
	}
   template <class T> VALUE & operator [] (const T&index) {
      return data[indexOf(index)];
   }
   template <class T> const VALUE & operator [] (const T&index) const{
      return data[indexOf(index)];
   }
};

#else
template <class VALUE, unsigned int dims, bool copy_data> class __BrtArray;

template <class VALUE, unsigned int dims, bool copy_data> class __ConstXSpecified {
	unsigned int indices [dims+1];
	const __BrtArray<VALUE,dims,copy_data> *const parent;
public:
	__ConstXSpecified(const __BrtArray<VALUE,dims,copy_data> *const parent):parent(parent) {
		indices[dims]=0;		
	}
	template <class T> __ConstXSpecified<VALUE,dims,copy_data> operator [] (const T &a) {
                int i=T::size-1;
                int cur_dimension=indices[dims];
                if (dims-cur_dimension<T::size)
                        i=dims-cur_dimension-1;
		for (;i>=0;--i,++cur_dimension){
			indices[cur_dimension]=(unsigned int)a.unsafeGetAt(i);
		}
                indices[dims]=cur_dimension;
		return *this;
	}
	const VALUE& gather()const;
	operator VALUE () const;
};
template <class VALUE, unsigned int dims, bool copy_data> class __XSpecified {
	unsigned int indices [dims+1];
	__BrtArray<VALUE,dims,copy_data> *parent;
public:
	__XSpecified(__BrtArray<VALUE,dims,copy_data> * parent) {
		indices[dims]=0;
		this->parent=parent;
	}
	template <class T> __XSpecified<VALUE,dims,copy_data> operator [] (const T &a) {
                int i=T::size-1;
                int cur_dimension=indices[dims];
                if (dims-cur_dimension<T::size)
                        i=dims-cur_dimension-1;
		for (;i>=0;--i,++cur_dimension){
			indices[cur_dimension]=(unsigned int)a.getAt(i);
		}
                indices[dims]=cur_dimension;
		return *this;
	}
/* nightmare	
	typedef typename GetValueOf<VALUE>::type TYPE;	
	TYPE getAt(unsigned int i)const { return GetAt((VALUE)*this,i);}
*/
	operator VALUE () const;
	VALUE& gather()const;	
	VALUE &operator =  (const VALUE &f)const;
	VALUE &operator += (const VALUE &f)const;
	VALUE &operator *= (const VALUE &f)const;
	VALUE &operator /= (const VALUE &f)const;
	VALUE &operator %= (const VALUE &f)const;
	VALUE &operator -= (const VALUE &f)const;	
};

template <class VALUE, unsigned int dims, bool copy_data> class __BrtArray {
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
	__BrtArray(VALUE * data, const unsigned int *extents) {
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
	__BrtArray& operator = (const __BrtArray<VALUE,dims,copy_data> &c) {
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
	__BrtArray(const __BrtArray <VALUE,dims,copy_data>&c) {
		*this=c;
	}
	~__BrtArray() {
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
	const VALUE &get(const unsigned int * indices)const {
		return data[this->linearaddresscalc(indices)];
	}
	VALUE& get (const unsigned int *indices) {
		return data[this->linearaddresscalc(indices)];
	}
	template <class T> __ConstXSpecified<VALUE,dims,copy_data> operator [] (const T &i) const{
		__ConstXSpecified<VALUE,dims,copy_data> ret(this);
		return ret[i];		
	}
	template <class T> __XSpecified<VALUE,dims,copy_data> operator [] (const T &i){
		__XSpecified<VALUE,dims,copy_data> ret(this);
		return ret[i];		
	}
	
};

template <class VALUE, unsigned int dims, bool copy_data> __XSpecified<VALUE,dims,copy_data>::operator VALUE ()const {
	return parent->get(indices);
}
template <class VALUE, unsigned int dims, bool copy_data> __ConstXSpecified<VALUE,dims,copy_data>::operator VALUE ()const {
	return parent->get(indices);
}
template <class VALUE, unsigned int dims, bool copy_data> VALUE &__XSpecified<VALUE,dims,copy_data>::gather ()const {
	return parent->get(indices);
}
template <class VALUE, unsigned int dims, bool copy_data> const VALUE & __ConstXSpecified<VALUE,dims,copy_data>::gather ()const {
	return parent->get(indices);
}

#define OPXD(op) template <class VALUE, unsigned int dims, bool copy_data>VALUE &__XSpecified<VALUE,dims,copy_data>::operator op (const VALUE &f)const{ \
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
#undef OPXD	

#endif
#endif
