// Implementing a lock-free ring buffer

// The following structure contains the user defined attributes of the ring buffer
// which will be passed into the initialization routine 

typedef struct {
    size_t s_elem;  // the size of each element
    size_t n_elem;  // the number of elements
    void *buffer;   // a pointer to the buffer which will hold the data
} rb_attr_t;        // The design of this structure means that the user must provide the memory used by the ring buffer to store the data

typedef unsigned int rbd_t; // This descriptor will be used by the caller to access the ring buffer which it has initialized.
                            // Its is an unsigned integer type because it will be used as an index into an array of the internal ring buffer structure

// The head and tail are all that is required for the next structure
struct ring_buffer
{
    size_t s_elem;
    size_t n_elem;
    uint8_t *buf;
    volatile size_t head; // the head and tail are both declared as volatile because they will be accessed from both the application context and the interrupt context
    volatile size_t tail; 
};                         //  The maximum number of ring buffers available in the system is determined at compile time by the hash define RING_BUFFER MAX

// The allocation of the ring buffer structure looks like this

static struct ring_buffer _rb[RING_BUFFER_MAX];

// The initialization of the ring buffer is straight forward.

int ring_buffer_init(rbd_t *rbd, rb_attr_t *attr)
{
    static int idx = 0;
    int err = -1; 
   
    /*we check that there is a free ring buffer, and that the rbd and attr pointers are not NULL*/
	/* The static variable ‘idx’ counts the number of used ring buffers*/
    if ((idx < RING_BUFFER_MAX) && (rbd != NULL) && (attr != NULL)) { 
	/* The second conditional statement verifies that the element size and buffer pointer are both valid*/
        if ((attr->buffer != NULL) && (attr->s_elem > 0)) {
            /* Check that the size of the ring buffer is a power of 2 */
            if (((attr->n_elem - 1) & attr->n_elem) == 0) {
                /* Initialize the ring buffer internal variables */
                _rb[idx].head = 0;
                _rb[idx].tail = 0;
                _rb[idx].buf = attr->buffer;
                _rb[idx].s_elem = attr->s_elem;
                _rb[idx].n_elem = attr->n_elem;
 
                *rbd = idx++;
                err= 0;
            }
        }
    }
 
    return err;
}

/* Now that all the arguments are validated, 
they are copied into the local structure and index is passed back to the caller as the ring buffer descriptor.
The variable idx is also incremented to indicate the ring buffer is used. 
The value will now be RING_BUFFER_MAX so if the initialization function is called again, it will fail. */

/*Before moving on to the rest of the public APIs, 
lets define the two static helper functions: _ring_buffer_full and _ring_buffer_empty.
Both calculate the difference between the head and the tail and then compare the result against the number of elements or zero respectively, 
they are incremented and wrap around automatically when they overflow.
This is a ‘feature’ of C (note this only applies to unsigned integers) 
and saves us from performing an additional calculation each time the function is called. 
It also allows us to calculate the number of elements currently in the ring buffer without any extra variables (read no counter = no critical section).
When the difference between the two is zero, the ring buffer is empty. 
However, since the head and tail are not wrapped around n_elem, 
so long as there is data in the ring buffer, the head and tail will never have the same value. 
The ring buffer is only full when the difference between the two is equal to n_elem.*/

static int _ring_buffer_full(struct ring_buffer *rb) /
{
    return ((rb->head - rb->tail) == rb->n_elem) ? 1 : 0;
}
 
static int _ring_buffer_empty(struct ring_buffer *rb)
{
    return ((rb->head - rb->tail) == 0U) ? 1 : 0;
}

/* The next function is ring_buffer_put which adds an element into the ring buffer.*/

int ring_buffer_put(rbd_t rbd, const void *data)
{
    int err = 0;
 
    if ((rbd < RING_BUFFER_MAX) && (_ring_buffer_full(&_rb[rbd]) == 0)) {
        const size_t offset = (_rb[rbd].head & (_rb[rbd].n_elem - 1)) * _rb[rbd].s_elem;
        memcpy(&(_rb[rbd].buf[offset]), data, _rb[rbd].s_elem);
        _rb[rbd].head++;
    } else {
        err = -1;
    }
 
    return err;
}

/* the size of each element is already known, the size of the data does not need to be passed in.
 After validating the argument and checking that the ring buffer is not full, the data needs to be copied into the ring buffer.
The offset into the buffer is determined by some more tricky math. 
The buffer is just an array of bytes so we need to know where each element starts in order to copy the data to the correct location. 
The head index must be wrapped around the number of elements in the ring buffer to obtain which element we want to write to. 
Typically, a wrapping operation is done using the modulus operation. For example, the offset could be calculated like this: */

const size_t offset = (_rb[rbd].head % _rb[rbd].n_elem) * _rb[rbd].s_elem;

/*The last function in this module is ring_buffer_get.*/

int ring_buffer_get(rbd_t rbd, void *data)
{
    int err = 0;
 
    if ((rbd < RING_BUFFER_MAX) && (_ring_buffer_empty(&_rb[rbd]) == 0)) {
        const size_t offset = (_rb[rbd].tail & (_rb[rbd].n_elem - 1)) * _rb[rbd].s_elem;
        memcpy(data, &(_rb[rbd].buf[offset]), _rb[rbd].s_elem);
        _rb[rbd].tail++;
    } else {
        err = -1;
    }
 
    return err;
}

/* It is essentially the same as ring_buffer_put, but instead of copying the data in, it is being copied out of the ring buffer back to the caller.
 The point at which the tail is incremented is key. In each of the previous two functions, only the head or tail is modified, never both.
 However, both values are read to determine the number of elements in the ring buffer. 
 To avoid having to use a critical section, the modification to the head must occur after reading the tail, and vise-versa.*/

 //Using the ring buffer in the UART driver
 
 /* the ring buffer descriptor_rbd and the ring buffer memory _rbmem must be declared*/

static rbd_t _rbd;
static char _rbmem[8];

/* In the initialization function uart_init, 
the ring buffer should be initialized by calling ring_buffer_init
 and passing the ring buffer attributes structure with each member assigned the values discussed.
 If the ring buffer initializes successfully, the UART module can be taken out of reset and the receive interrupt is enabled in IFG2.*/
 
 ...
if (i < ARRAY_SIZE(_baud_tbl)) {
    rb_attr_t attr = {sizeof(_rbmem[0]), ARRAY_SIZE(_rbmem), _rbmem};
 
    /* Set the baud rate */
    UCA0BR0 = _baud_tbl[i].UCAxBR0;
    UCA0BR1 = _baud_tbl[i].UCAxBR1;
    UCA0MCTL = _baud_tbl[i].UCAxMCTL;
 
    /* Initialize the ring buffer */
    if (ring_buffer_init(&_rbd, &attr) == 0) {
        /* Enable the USCI peripheral (take it out of reset) */
        UCA0CTL1 &= ~UCSWRST;
 
        /* Enable rx interrupts */
        IE2 |= UCA0RXIE;
 
        status = 0;
    }
}
...

/* The second function that must be modified is uart_getchar.
 Reading the received character out of the UART peripheral is replaced by reading from the queue. 
 If the queue is empty, the function should return -1 as it did before.*/
 
 int uart_getchar(void)
{
    char c = -1;
 
    ring_buffer_get(_rbd, &c);
 
    return c;
}

//Finally, we need to implement the UART receive ISR

__attribute__((interrupt(USCIAB0RX_VECTOR))) void rx_isr(void)
{
    if (IFG2 & UCA0RXIFG) {
        const char c = UCA0RXBUF;
 
        /* Clear the interrupt flag */
        IFG2 &= ~UCA0RXIFG;
 
        ring_buffer_put(_rbd, &c);
    }
}
