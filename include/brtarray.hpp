#ifndef BRTARRAY_H_
#define BRTARRAY_H_
#ifndef MULTIPLE_ARRAY_BOUNDS_LOOKUPS
#define ARRAY_EPSILON .25

template <class W> unsigned int clampArrayBound(W val, unsigned int extent) {
    val=(W)floor(val+ARRAY_EPSILON);
    assert(val>-80000&&val<extent+80000);
    return val<0?0:((unsigned int)val)>=extent?extent-1:(unsigned int)val;
}

template <class VALUE> class __BrtArray {
  unsigned int dims;
  unsigned int elemsize;
  unsigned int  * extents;
  unsigned char * data;
  brook::Stream * s;

  unsigned int getSize() const{
    unsigned int size=1;
    for (unsigned int i=0;i<dims;++i) {
      size*=extents[i];
    }
    return size;
  }

  void init(VALUE * data, unsigned int dims, unsigned int elemsize, 
            const unsigned int *extents) {
    
    this->dims = dims;
    this->elemsize = elemsize;
    this->extents = (unsigned int *) malloc (dims * sizeof(unsigned int));

    for (unsigned int i=0;i<dims;++i) {
      this->extents[i]=extents[i];
    }		

    this->data = (unsigned char *) data;
  }
  
public:
  __BrtArray(VALUE * data, unsigned int dims, unsigned int elemsize, 
             const unsigned int *extents):s(0) {
    init(data,dims,elemsize,extents);
  }

  __BrtArray(brook::Stream * stm):s(0) {
    init((VALUE *)stm->getData(brook::Stream::READ),
         stm->getDimension(),
         stm->getElementSize(),
         stm->getExtents());
    this->s=stm;
  }

  __BrtArray& operator = (const __BrtArray<VALUE> &c) {

    if (dims != c.dims) {
      fprintf(stderr, "CPU: Assignment of mismatching arrays");
      exit(1);
    }
    
    for (unsigned int i=0;i<dims;++i) {
      extents[i]=c.extents[i];
    }

    this->dims = c.dims;
    this->elemsize = c.elemsize;
    this->data = c.data;

    brook::Stream * tmpstr=this->s;
    if (c.s) {
      this->s=c.s;
      this->data=this->s->getData(brook::Stream::READ);
    }
    if (tmpstr) {
      tmpstr->releaseData(brook::Stream::READ);
    }
 
  return *this;
  }

  __BrtArray(const __BrtArray <VALUE>&c):s(0){

    if (dims != c.dims) {
      fprintf(stderr, "CPU: Copy of mismatching arrays");
      exit(1);
    }

    *this=c;
  }

  ~__BrtArray() {
    if (s)
      s->releaseData(brook::Stream::READ);
    free(extents);
  }

  template <class T> unsigned int indexOf (const T &index) const {
    unsigned int i=T::size-1;

    if (dims<T::size)
      i=dims-1;

    float temp = (float)floor(ARRAY_EPSILON+index.getAt(i));
    unsigned int total=(temp<0)?0:temp>=extents[0]-1?extents[0]-1:
      (unsigned int)temp;

    for (unsigned int j=1;j<=i;++j) {
      total*=extents[j];
      temp = (float)floor(ARRAY_EPSILON+index.getAt(i-j));
      total+=(temp<0)?0:temp>=extents[j]-1?extents[j]-1:
        (unsigned int)temp;
    }
    return total;
  }

  template <class T> VALUE& getInBounds (const T &index) {
    static VALUE emergency;

    unsigned int i=T::size-1;

    if (dims<T::size)
      i=dims-1;
    float temp = (float)floor(ARRAY_EPSILON+index.getAt(i));
    if (!(temp>=0&&temp<extents[0]))
      return emergency;
    
    unsigned int total=(unsigned int)temp;
    for (unsigned int j=1;j<=i;++j) {
      total*=extents[j];
      temp = (float)floor(ARRAY_EPSILON+index.getAt(i-j));
      if (!(temp>=0&&temp<extents[j]))
        return emergency;
      total+=(unsigned int)temp;
    }

    return data[total];
  }

  template <class T> VALUE & operator [] (const T & index) {
    return *(VALUE *) (data+indexOf(index) * elemsize);
  }
  template <class T> const VALUE & operator [] (const T & index) const{
    return *(VALUE *) (data+indexOf(index) * elemsize);
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
