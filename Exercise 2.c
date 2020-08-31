// Memory allocation using Pool
/* Dynamic memory allocation is a method which reserves RAM memory space (heap) in run-time code execution. 
In contrast to static memory allocation, it brings new possibilities in case of writing embedded programs.
First of all, a Pool is a container that comprises declared SIZE number of elements.
Each element has the same byte size. The application can take (allocate) elements from the Pool, use it and when it is not needed anymore,
 give it back to the Pool (deallocate). Other variation of the Pool pattern is when deallocation is not supported.
 It is useful for the application which uses the Pool only at system initialization (startup). 
 This is the safest version of the dynamic memory allocation, 
 because it results in zero memory leakage, reasonable memory waste and no fragmentation.*/
 
 // API of the Pool pattern
 
 namespace k2lib
{
// namespace k2lib body

template <typename T, int SIZE>
class Pool
{
public:
  Pool();
  T *palloc();     /*!< Take (allocate) one element from the Pool. */
  void free(T *p); /*!< Give back (free) element of given argument pointer. */
  int size();      /*!< Returns count of free Pool elements. */

private:
  /* Rest of the elements... */
};

} // namespace k2lib

// Set/clear/test bits in an array.

/* In order to calculate position of the bit given by bit_index argument,
you have to calculate byte position byte_offset in the info array and bit position in the target byte bit_index.
 Now, you can get &info[0] address and move it by byte_offset. 
 Obtained array element can be set/cleared/tested with the calculated bit_index - 1 << bit_index.*/

void setBit(int bit_index)
{
  if (bit_index >= SIZE)
    return;

  int byte_offset = (bit_index / BITS_IN_UINT8);
  bit_index = (bit_index % BITS_IN_UINT8);
  *((uint8_t *)(&info[0] + byte_offset)) |= (uint8_t)(1 << bit_index);
}

void clrBit(int bit_index)
{
  if (bit_index >= SIZE)
    return;

  int byte_offset = (bit_index / BITS_IN_UINT8);
  bit_index = (bit_index % BITS_IN_UINT8);
  *((uint8_t *)(&info[0] + byte_offset)) &= ~((uint8_t)(1 << bit_index));
}

bool testBit(int bit_index)
{
  if (bit_index >= SIZE)
    return false;

  int byte_offset = (bit_index / BITS_IN_UINT8);
  bit_index = (bit_index % BITS_IN_UINT8);
  return (0 !=
          (*((uint8_t *)(&info[0] + byte_offset)) & (uint8_t)(1 << bit_index)));
}

// complete implementation

#ifndef POOL_H
#define POOL_H

#include <stddef.h>
#include <stdint.h>


namespace k2lib
{
// namespace k2lib body

template <typename T, int SIZE>
class Pool
{
public:
  Pool();
  T *palloc();     /*!< Take (allocate) one element from the Pool. */
  void free(T *p); /*!< Give back (free) element of given argument pointer. */
  int size();      /*!< Returns count of free Pool elements. */

private:
  T elements[SIZE]; /*!< Holds the pool objects. */

  static const size_t BITS_IN_UINT8 = 8; /*!< Number of bits in uint8_t type. */
  static const size_t NO_BYTES =
      (SIZE + BITS_IN_UINT8 - 1) /
      BITS_IN_UINT8;      /*!< Required number of bytes to hold information
                          about free/allocated pool objects. */
  uint8_t info[NO_BYTES]; /*!< Keeps track of free/allocated memory objects. */
  int free_elements_cnt;  /*!< Counts number of free elements */

  bool testBit(int bit_index);
  void setBit(int bit_index);
  void clrBit(int bit_index);
};


template <typename T, int SIZE>
Pool<T, SIZE>::Pool()
{
  // Free all pool objects
  for (unsigned int i = 0; i < NO_BYTES; i++)
    info[i] = 0xFFU;
  // Clera
  free_elements_cnt = SIZE;
}

template <typename T, int SIZE>
T *Pool<T, SIZE>::palloc()
{
  // Test whether there are any free elements
  if (free_elements_cnt <= 0)
    return NULL;

  // Look up for first pool object
  for (int i = 0; i < SIZE; i++)
  {
    if (testBit(i))
    {
      // Found free element, so mark it as allocated and return
      clrBit(i);
      free_elements_cnt--;
      return &elements[i];
    }
  }

  // No free elements available
  return NULL;
}

template <typename T, int SIZE>
void Pool<T, SIZE>::free(T *p)
{
  // Cannot free NULL pointer
  if (NULL == p)
    return;

  // Loop through all elements and find the one to be freed
  for (int i = 0; i < SIZE; i++)
  {
    if (p == &elements[i])
    {
      // Element found, so free this element
      p = NULL;
      setBit(i);
      free_elements_cnt++;
    }
  }

  // No elements found, memory cannot be freed
  return;
}

template <typename T, int SIZE>
int Pool<T, SIZE>::size()
{
  return free_elements_cnt;
}

template <typename T, int SIZE>
bool Pool<T, SIZE>::testBit(int bit_index)
{
  if (bit_index >= SIZE)
    return false;

  int byte_offset = (bit_index / BITS_IN_UINT8);
  bit_index = (bit_index % BITS_IN_UINT8);
  return (0 !=
          (*((uint8_t *)(&info[0] + byte_offset)) & (uint8_t)(1 << bit_index)));
}

template <typename T, int SIZE>
void Pool<T, SIZE>::setBit(int bit_index)
{
  if (bit_index >= SIZE)
    return;

  int byte_offset = (bit_index / BITS_IN_UINT8);
  bit_index = (bit_index % BITS_IN_UINT8);
  *((uint8_t *)(&info[0] + byte_offset)) |= (uint8_t)(1 << bit_index);
}

template <typename T, int SIZE>
void Pool<T, SIZE>::clrBit(int bit_index)
{
  if (bit_index >= SIZE)
    return;

  int byte_offset = (bit_index / BITS_IN_UINT8);
  bit_index = (bit_index % BITS_IN_UINT8);
  *((uint8_t *)(&info[0] + byte_offset)) &= ~((uint8_t)(1 << bit_index));
}

} // namespace k2lib

#endif /* end of include guard: POOL_H */
