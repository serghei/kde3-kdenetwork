#include "simplelist.h"

template <class T>
SimpleList<T>::SimpleList()
   :m_list(0)
   ,m_current(0)
   ,m_last(0)
   ,m_size(0)
{};

template <class T>
SimpleList<T>::~SimpleList()
{
   clear();
};

template <class T>
void SimpleList<T>::append(const T& text)
{
   if (m_list==0)
   {
      m_list=new TemplNode<T>(text);
      m_last=m_list;
   }
   else
   {
      m_last->m_next=new TemplNode<T>(text);
      m_last=m_last->m_next;
   };
   m_size++;
};

template <class T>
void SimpleList<T>::removeFirst()
{
   if (m_list==0) return;
   TemplNode<T> *first=m_list;
   m_list=m_list->m_next;
   m_size--;
   if (m_list==0)
      m_last=0;
   m_current=0;
   delete first;
};

template <class T>
void SimpleList<T>::clear()
{
   while (m_list!=0)
      removeFirst();
   m_current=0;
   m_last=0;
   m_list=0;
   m_size=0;
};

template <class T>
void SimpleList<T>::remove(T* item)
{
   if (item==0) return;
   TemplNode<T>* pre(0);
   for (T* tmp=first(); tmp!=0; tmp=next())
   {
      if (tmp==item)
      {
         if (m_current==m_list)
         {
            removeFirst();
            return;
         }
         else
         {
            TemplNode<T> *succ=m_current->m_next;
            if (m_current==m_last)
               m_last=pre;
            delete m_current;
            pre->m_next=succ;
            m_size--;
            m_current=0;
            
         };
      };
      pre=m_current;
   };
   
};

template <class T>
T* SimpleList<T>::first()
{
   m_current=m_list;
   if (m_list==0)
      return 0;
   return &m_current->m_item;
};

template <class T>
T* SimpleList<T>::next()
{
   if (m_current==0) return 0;
   m_current=m_current->m_next;
   if (m_current==0) return 0;
   return &m_current->m_item;
};

template <class T>
int SimpleList<T>::size()
{
   return m_size;
}
