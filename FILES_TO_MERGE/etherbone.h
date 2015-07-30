/** @file etherbone.h
 *  @brief The public API of the Etherbone library.
 *
 *  Copyright (C) 2011-2012 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 *  All Etherbone object types are opaque in this interface.
 *  Only those methods listed in this header comprise the public interface.
 *
 *  @author Wesley W. Terpstra <w.terpstra@gsi.de>
 *
 *  @bug None!
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

#ifndef ETHERBONE_H
#define ETHERBONE_H

#define EB_PROTOCOL_VERSION	1
#define EB_ABI_VERSION		0x04	/* incremented on incompatible changes */

#include <stdint.h>   /* uint32_t ... */
#include <inttypes.h> /* EB_DATA_FMT ... */

/* Symbol visibility definitions */
#ifdef __WIN32
#ifdef ETHERBONE_IMPL
#define EB_PUBLIC __declspec(dllexport)
#define EB_PRIVATE
#else
#define EB_PUBLIC __declspec(dllimport)
#define EB_PRIVATE
#endif
#else
#define EB_PUBLIC
#define EB_PRIVATE __attribute__((visibility("hidden")))
#endif

/* Pointer type -- depends on memory implementation */
#ifdef EB_USE_MALLOC
#define EB_POINTER(typ) struct typ*
#define EB_NULL 0
#define EB_MEMORY_MODEL 0x0001U
#else
#define EB_POINTER(typ) uint16_t
#define EB_NULL ((uint16_t)-1)
#define EB_MEMORY_MODEL 0x0000U
#endif

/* Opaque structural types */
typedef EB_POINTER(eb_socket)    eb_socket_t;
typedef EB_POINTER(eb_device)    eb_device_t;
typedef EB_POINTER(eb_cycle)     eb_cycle_t;
typedef EB_POINTER(eb_operation) eb_operation_t;

/* Configurable maximum bus width supported */
#if defined(EB_FORCE_64)
typedef uint64_t eb_address_t;
typedef uint64_t eb_data_t;
#define EB_ADDR_FMT PRIx64
#define EB_DATA_FMT PRIx64
#define EB_DATA_C UINT64_C
#define EB_ADDR_C UINT64_C
#elif defined(EB_FORCE_32)
typedef uint32_t eb_address_t;
typedef uint32_t eb_data_t;
#define EB_ADDR_FMT PRIx32
#define EB_DATA_FMT PRIx32
#define EB_DATA_C UINT32_C
#define EB_ADDR_C UINT32_C
#elif defined(EB_FORCE_16)
typedef uint16_t eb_address_t;
typedef uint16_t eb_data_t;
#define EB_ADDR_FMT PRIx16
#define EB_DATA_FMT PRIx16
#define EB_DATA_C UINT16_C
#define EB_ADDR_C UINT16_C
#else
/* The default maximum width is the machine word-size */
typedef uintptr_t eb_address_t;
typedef uintptr_t eb_data_t;
#define EB_ADDR_FMT PRIxPTR
#define EB_DATA_FMT PRIxPTR
#define EB_DATA_C UINT64_C
#define EB_ADDR_C UINT64_C
#endif

/* Identify the library ABI this header must match */
#define EB_BUS_MODEL	(0x0010U * sizeof(eb_address_t)) + (0x0001U * sizeof(eb_data_t))
#define EB_ABI_CODE	((EB_ABI_VERSION << 8) + EB_BUS_MODEL + EB_MEMORY_MODEL)

/* Status codes; format using eb_status() */
typedef int eb_status_t;
#define EB_OK         0  /* success */
#define EB_FAIL      -1  /* system failure */
#define EB_ADDRESS   -2  /* invalid address */
#define EB_WIDTH     -3  /* impossible bus width */
#define EB_OVERFLOW  -4  /* cycle length overflow */
#define EB_ENDIAN    -5  /* remote endian required */
#define EB_BUSY      -6  /* resource busy */
#define EB_TIMEOUT   -7  /* timeout */
#define EB_OOM       -8  /* out of memory */
#define EB_ABI       -9  /* library incompatible with application */
#define EB_SEGFAULT  -10 /* one or more operations failed */

/* A bitmask containing values from EB_DATAX | EB_ADDRX */
typedef uint8_t eb_width_t;
/* A bitmask containing values from EB_DATAX | EB_ENDIAN_MASK */
typedef uint8_t eb_format_t;

#define EB_DATA8	0x01
#define EB_DATA16	0x02
#define EB_DATA32	0x04
#define EB_DATA64	0x08
#define EB_DATAX	0x0f

#define EB_ADDR8	0x10
#define EB_ADDR16	0x20
#define EB_ADDR32	0x40
#define EB_ADDR64	0x80
#define EB_ADDRX	0xf0

#define EB_ENDIAN_MASK	0x30
#define	EB_BIG_ENDIAN	0x10
#define EB_LITTLE_ENDIAN 0x20

#define EB_DESCRIPTOR_IN  0x01
#define EB_DESCRIPTOR_OUT 0x02

/* Callback types */
typedef void *eb_user_data_t;
typedef void (*eb_callback_t)(eb_user_data_t, eb_device_t, eb_operation_t, eb_status_t);

/* Special callbacks */
#define eb_block 0

typedef int eb_descriptor_t;
typedef int (*eb_descriptor_callback_t)(eb_user_data_t, eb_descriptor_t, uint8_t mode); /* mode = EB_DESCRIPTOR_IN | EB_DESCRIPTOR_OUT */

/* Type of the SDB record */
enum sdb_record_type {
  sdb_record_interconnect = 0x00,
  sdb_record_device       = 0x01,
  sdb_record_bridge       = 0x02,
  sdb_record_integration  = 0x80,
  sdb_record_empty        = 0xFF,
};

/* The type of bus (specifies bus-specific fields) */
enum sdb_bus_type {
  sdb_wishbone = 0x00
};

/* 40 bytes, 8-byte alignment */
struct sdb_product {
  uint64_t vendor_id;   /* Vendor identifier */
  uint32_t device_id;   /* Vendor assigned device identifier */
  uint32_t version;     /* Device-specific version number */
  uint32_t date;        /* Hex formatted release date (eg: 0x20120501) */
  int8_t   name[19];    /* ASCII. no null termination. */
  uint8_t  record_type; /* sdb_record_type */
};

/* 56 bytes, 8-byte alignment */
struct sdb_component {
  uint64_t           addr_first; /* Address range: [addr_first, addr_last] */
  uint64_t           addr_last;
  struct sdb_product product;
};

/* Record type: sdb_empty */
struct sdb_empty {
  int8_t  reserved[63];
  uint8_t record_type;
};

/* Record type: sdb_interconnect
 * This header prefixes every SDB table.
 * It's component describes the interconnect root complex/bus/crossbar.
 */
struct sdb_interconnect {
  uint32_t             sdb_magic;    /* 0x5344422D */
  uint16_t             sdb_records;  /* Length of the SDB table (including header) */
  uint8_t              sdb_version;  /* 1 */
  uint8_t              sdb_bus_type; /* sdb_bus_type */
  struct sdb_component sdb_component;
};

/* Record type: sdb_integration
 * This meta-data record describes the aggregate product of the bus.
 * For example, consider a manufacturer which takes components from 
 * various vendors and combines them with a standard bus interconnect.
 * The integration component describes aggregate product.
 */
struct sdb_integration {
  int8_t             reserved[24];
  struct sdb_product product;
};

/* Flags used for wishbone bus' in the bus_specific field */
#define SDB_WISHBONE_WIDTH          0xf
#define SDB_WISHBONE_LITTLE_ENDIAN  0x80

/* Record type: sdb_device
 * This component record describes a device on the bus.
 * abi_class describes the published standard register interface, if any.
 */
struct sdb_device {
  uint16_t             abi_class; /* 0 = custom device */
  uint8_t              abi_ver_major;
  uint8_t              abi_ver_minor;
  uint32_t             bus_specific;
  struct sdb_component sdb_component;
};

/* Record type: sdb_bridge
 * This component describes a bridge which embeds a nested bus.
 * This does NOT include bus controllers, which are *devices* that
 * indirectly control a nested bus.
 */
struct sdb_bridge {
  uint64_t             sdb_child; /* Nested SDB table */
  struct sdb_component sdb_component;
};

/* All possible SDB record structure */
union sdb_record {
  struct sdb_empty        empty;
  struct sdb_device       device;
  struct sdb_bridge       bridge;
  struct sdb_integration  integration;
  struct sdb_interconnect interconnect;
};

/* Complete bus description */
struct sdb_table {
  struct sdb_interconnect interconnect;
  union  sdb_record       record[1]; /* bus.sdb_records-1 elements (not 1) */
};

/* Handler descriptor */
struct eb_handler {
  /* This pointer must remain valid until after you detach the device */
  const struct sdb_device* device;
  
  eb_user_data_t data;
  eb_status_t (*read) (eb_user_data_t, eb_address_t, eb_width_t, eb_data_t*);
  eb_status_t (*write)(eb_user_data_t, eb_address_t, eb_width_t, eb_data_t);
};

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/*                                 C99 API                                  */
/****************************************************************************/

/* Convert status to a human-readable printable string */
EB_PUBLIC
const char* eb_status(eb_status_t code);

/* Convert data width mask to a string (8/16/32/64) */
EB_PUBLIC
const char* eb_width_data(eb_width_t width);

/* Convert address width mask to a string (8/16/32/64) */
EB_PUBLIC
const char* eb_width_address(eb_width_t width);

/* Convert format's data width mask to a string (8/16/32/64) */
EB_PUBLIC
const char* eb_format_data(eb_width_t width);

/* Convert the endian format to a string */
EB_PUBLIC
const char* eb_format_endian(eb_format_t format);

/* Convert a string to a mask (EB_FAIL/EB_WIDTH returned) */
EB_PUBLIC
int eb_width_parse_data(const char* str, eb_width_t* width);

/* Convert a string to a mask (EB_FAIL/EB_WIDTH returned) */
EB_PUBLIC
int eb_width_parse_address(const char* str, eb_width_t* width);

/* Open an Etherbone socket for communicating with remote devices.
 * Open sockets must be hooked into an event loop; see eb_socket_{run,check}.
 * 
 * The abi_code must be EB_ABI_CODE. This confirms library compatability.
 * The port parameter is optional; 0 lets the operating system choose.
 * Supported_widths list bus widths acceptable to the local Wishbone bus.
 *   EB_ADDR32|EB_ADDR8|EB_DATAX means 8/32-bit addrs and 8/16/32/64-bit data.
 *   Devices opened by this socket only negotiate a subset of these widths.
 *   Virtual slaves attached to the socket never see a width excluded here.
 *
 * Return codes:
 *   OK		- successfully open the socket port
 *   FAIL	- operating system forbids access
 *   BUSY	- specified port is in use (only possible if port != 0)
 *   WIDTH      - supported_widths were invalid
 *   OOM        - out of memory
 *   ABI        - library is not compatible with application
 */
EB_PUBLIC
eb_status_t eb_socket_open(uint16_t      abi_code,
                           const char*   port, 
                           eb_width_t    supported_widths,
                           eb_socket_t*  result);

/* Close the Etherbone socket.
 * Any use of the socket after successful close will probably segfault!
 *
 * Return codes:
 *   OK		- successfully closed the socket and freed memory
 *   BUSY	- there are open devices on this socket
 */
EB_PUBLIC
eb_status_t eb_socket_close(eb_socket_t socket);

/* Wait for an event on the socket and process it.
 * This function is useful if your program has no event loop of its own.
 * If timeout_us == 0, return immediately. If timeout_us == -1, wait forever.
 * It returns the time expended while waiting.
 */
EB_PUBLIC
long eb_socket_run(eb_socket_t socket, long timeout_us);

/* Integrate this Etherbone socket into your own event loop.
 *
 * You must call eb_socket_check whenever:
 *   1. An etherbone timeout expires (eb_socket_timeout tells you when this is)
 *   2. An etherbone socket is ready to read (eb_socket_descriptors lists them)
 * You must provide eb_socket_check with:
 *   1. The current time
 *   2. A function that returns '1' if a socket is ready to read
 *
 * YOU MAY NOT CLOSE OR MODIFY ETHERBONE SOCKET DESCRIPTORS IN ANY WAY.
 */
EB_PUBLIC
int eb_socket_check(eb_socket_t socket, uint32_t now, eb_user_data_t user, eb_descriptor_callback_t ready);

/* Calls (*list)(user, fd) for every descriptor the socket uses. */
EB_PUBLIC
void eb_socket_descriptors(eb_socket_t socket, eb_user_data_t user, eb_descriptor_callback_t list); 

/* Returns 0 if there are no timeouts pending, otherwise the time in UTC seconds. */
EB_PUBLIC
uint32_t eb_socket_timeout(eb_socket_t socket);

/* Add a device to the virtual bus.
 * This handler receives all reads and writes to the specified address.
 * The handler structure passed to eb_socket_attach need not be preserved.
 * The nested sdb_device MUST be preserved until the device is detached.
 * NOTE: the address range [0x0, 0x4000) is reserved for internal use.
 *
 * Return codes:
 *   OK         - the handler has been installed
 *   OOM        - out of memory
 *   ADDRESS    - the specified address range overlaps an existing device.
 */
EB_PUBLIC
eb_status_t eb_socket_attach(eb_socket_t socket, const struct eb_handler* handler);

/* Detach the device from the virtual bus.
 *
 * Return codes:
 *   OK         - the devices has be removed
 *   ADDRESS    - there is no device at the specified address.
 */
EB_PUBLIC
eb_status_t eb_socket_detach(eb_socket_t socket, const struct sdb_device* device);

/* Open a remote Etherbone device at 'address' (default port 0xEBD0).
 * Negotiation of bus widths is attempted every 3 seconds, 'attempts' times.
 * The proposed_widths is intersected with the remote and local socket widths.
 * From the remaining widths, the largest address and data width is chosen.
 *
 * Return codes:
 *   OK		- the remote etherbone device is ready
 *   ADDRESS	- the network address could not be parsed
 *   TIMEOUT    - timeout waiting for etherbone response
 *   FAIL       - failure of the transport layer (remote host down?)
 *   WIDTH      - could not negotiate an acceptable data bus width
 *   OOM        - out of memory
 */
EB_PUBLIC
eb_status_t eb_device_open(eb_socket_t  socket,
                           const char*  address,
                           eb_width_t   proposed_widths,
                           int          attempts,
                           eb_device_t* result);

/* The same method using a non-blocking interface.
 * The callback receives the device and final status.
 */
EB_PUBLIC
eb_status_t eb_device_open_nb(eb_socket_t    socket,
                              const char*    address,
                              eb_width_t     proposed_widths,
                              eb_user_data_t user_data,
                              eb_callback_t  callback);

/* Open a remote Etherbone device at 'address' (default port 0xEBD0) passively.
 * The channel is opened and the remote device should initiate the EB exchange.
 * This is useful for stream protocols where the master cannot be the initiator.
 *
 * Return codes:
 *   OK		- the remote etherbone device is ready
 *   ADDRESS	- the network address could not be parsed
 *   TIMEOUT    - timeout waiting for etherbone response
 *   FAIL       - failure of the transport layer (remote host down?)
 *   WIDTH      - could not negotiate an acceptable data bus width
 *   OOM        - out of memory
 */
EB_PUBLIC
eb_status_t eb_socket_passive(eb_socket_t           socket, 
                              const char*           address);


/* Recover the negotiated port and address width of the target device. */
EB_PUBLIC
eb_width_t eb_device_width(eb_device_t device);

/* Get the interrupt range of the target device. */
eb_status_t eb_device_get_irq_range(eb_device_t devicep, 
                                    eb_data_t*  irq_start_high, 
                                    eb_data_t*  irq_start_low, 
                                    eb_data_t*  irq_end_high, 
                                    eb_data_t*  irq_end_low);

/* Close a remote Etherbone device.
 * Any inflight or ready-to-send cycles will receive EB_TIMEOUT.
 *
 * Return codes:
 *   OK	        - associated memory has been freed
 *   BUSY       - there are unclosed wishbone cycles on this device
 */
EB_PUBLIC
eb_status_t eb_device_close(eb_device_t device);

/* Access the socket backing this device */
EB_PUBLIC
eb_socket_t eb_device_socket(eb_device_t device);

/* Begin a wishbone cycle on the remote device.
 * Read/write operations within a cycle hold the device locked.
 * Read/write operations are executed in the order they are queued.
 * Until the cycle is closed and main loop runs, the operations are not sent.
 *
 * Returns:
 *    FAIL      - device is being closed, cannot create new cycles
 *    OOM       - insufficient memory
 *    OK        - cycle created successfully (your callback will be run)
 * 
 * Your callback will be called exactly once from either:
 *   eb_socket_{run,check} or eb_device_close or a blocking eb_cycle_close
 * It receives these arguments: cb(user_data, device, operations, status)
 * 
 * If status != OK, the cycle was never sent to the remote bus.
 * If status == OK, the cycle was sent.
 *
 * When status == EB_OK, 'operations' report the wishbone ERR flag.
 * When status != EB_OK, 'operations' points to the offending operation.
 *
 * Callback status codes:
 *   OK         - cycle was executed successfully
 *   ADDRESS    - 1. a specified address exceeded device bus address width
 *                2. the address was not aligned to the operation granularity
 *   WIDTH      - 1. written value exceeded the operation granularity
 *                2. the granularity exceeded the device port width
 *   ENDIAN     - operation format was not word size and no endian was specified
 *   OVERFLOW   - too many operations queued for this cycle (wire limit)
 *   TIMEOUT    - remote system never responded to EB request
 *   FAIL       - remote host violated protocol
 *   OOM        - out of memory while queueing operations to the cycle
 *   SEGFAULT   - at least one operation reported ERR not ACK (see eb_operation_had_error)
 */
EB_PUBLIC
eb_status_t eb_cycle_open(eb_device_t    device, 
                          eb_user_data_t user_data,
                          eb_callback_t  cb,
                          eb_cycle_t*    result);

/* End a wishbone cycle.
 * This places the complete cycle at end of the device's send queue.
 * Unless eb_block was used, returns EB_OK. Otherwise final status.
 */
EB_PUBLIC
eb_status_t eb_cycle_close(eb_cycle_t cycle);

/* End a wishbone cycle.
 * This places the complete cycle at end of the device's send queue.
 * This method does NOT check individual wishbone operation error status.
 */
EB_PUBLIC
eb_status_t eb_cycle_close_silently(eb_cycle_t cycle);

/* End a wishbone cycle.
 * The cycle is discarded, freed, and the callback never invoked.
 */
EB_PUBLIC
void eb_cycle_abort(eb_cycle_t cycle);

/* Access the device targetted by this cycle */
EB_PUBLIC
eb_device_t eb_cycle_device(eb_cycle_t cycle);

/* Prepare a wishbone read operation.
 * The given address is read from the remote device.
 * The result is written to the data address.
 * If data == 0, the result can still be accessed via eb_operation_data.
 *
 * The operation size is max {x in format: x <= data_width(device) }.
 * When the size is not the device data width, format must include an endian.
 * Your address must be aligned to the operation size.
 */
EB_PUBLIC
void eb_cycle_read(eb_cycle_t    cycle, 
                   eb_address_t  address,
                   eb_format_t   format,
                   eb_data_t*    data);
EB_PUBLIC
void eb_cycle_read_config(eb_cycle_t    cycle, 
                          eb_address_t  address,
                          eb_format_t   format,
                          eb_data_t*    data);

/* Perform a wishbone write operation.
 * The given address is written on the remote device.
 * 
 * The operation size is max {x in width: x <= data_width(device) }.
 * When the size is not the device data width, format must include an endian.
 * Your address must be aligned to this operation size and the data must fit.
 */
EB_PUBLIC
void eb_cycle_write(eb_cycle_t    cycle,
                    eb_address_t  address,
                    eb_format_t   format,
                    eb_data_t     data);
EB_PUBLIC
void eb_cycle_write_config(eb_cycle_t    cycle,
                           eb_address_t  address,
                           eb_format_t   format,
                           eb_data_t     data);

/* Operation result accessors */

/* The next operation in the list. EB_NULL = end-of-list */
EB_PUBLIC eb_operation_t eb_operation_next(eb_operation_t op);

/* Was this operation a read? 1=read, 0=write */
EB_PUBLIC int eb_operation_is_read(eb_operation_t op);
/* Was this operation onthe config space? 1=config, 0=wb-bus */
EB_PUBLIC int eb_operation_is_config(eb_operation_t op);
/* Did this operation have an error? 1=error, 0=success */
EB_PUBLIC int eb_operation_had_error(eb_operation_t op);
/* What was the address of this operation? */
EB_PUBLIC eb_address_t eb_operation_address(eb_operation_t op);
/* What was the read or written value of this operation? */
EB_PUBLIC eb_data_t eb_operation_data(eb_operation_t op);
/* What was the format of this operation? */
EB_PUBLIC eb_format_t eb_operation_format(eb_operation_t op);

/* Convenience methods which create single-operation cycles.
 * Equivalent to: eb_cycle_open, eb_cycle_{read,write}, eb_cycle_close.
 */
EB_PUBLIC eb_status_t eb_device_read(eb_device_t    device,
                                     eb_address_t   address,
                                     eb_format_t    format,
                                     eb_data_t*     data,
                                     eb_user_data_t user_data,
                                     eb_callback_t  cb);

EB_PUBLIC eb_status_t eb_device_write(eb_device_t    device,
                                      eb_address_t   address,
                                      eb_format_t    format,
                                      eb_data_t      data,
                                      eb_user_data_t user_data,
                                      eb_callback_t  cb);

/* Read the SDB information from the remote bus.
 * If there is not enough memory to initiate the request, EB_OOM is returned.
 * To scan the root bus, Etherbone config space is used to locate the SDB record.
 * When scanning a child bus, supply the bridge's sdb_device record.
 *
 * All fields in the processed structures are in machine native endian.
 * When scanning a child bus, nested addresses are automatically converted.
 *
 * Your callback is called from eb_socket_{run,check} or eb_device_close or a blocking eb_cycle_close.
 * It receives these arguments: (user_data, device, sdb, status)
 *
 * If status != OK, the SDB information could not be retrieved.
 * If status == OK, the structure was retrieved.
 *
 * The sdb object passed to your callback is only valid until you return.
 * If you need persistent information, you must copy the memory yourself.
 */
typedef void (*sdb_callback_t)(eb_user_data_t, eb_device_t device, const struct sdb_table*, eb_status_t);
EB_PUBLIC eb_status_t eb_sdb_scan_bus(eb_device_t device, const struct sdb_bridge* bridge, eb_user_data_t data, sdb_callback_t cb);
EB_PUBLIC eb_status_t eb_sdb_scan_root(eb_device_t device, eb_user_data_t data, sdb_callback_t cb);

/* Convenience methods for locating / identifying devices.
 * These calls are blocking! If you need the power API, use the above methods.
 */

/* Find the SDB record for a device at address.
 * If no matching record is found EB_ADDRESS is returned.
 * On success, EB_OK is returned.
 * Other failures are possible.
 */
EB_PUBLIC eb_status_t eb_sdb_find_by_address(eb_device_t device, eb_address_t address, struct sdb_device* output);

/* When scanning by identity, multiple devices may match.
 * The initial value of *devices determines the output space in records.
 * The resulting value of *devices is the number of matching SDB records found.
 * If there are more records than fit in output, they are counted but unwritten.
 * On success, EB_OK is returned. Do not forget to check *devices as well!
 */
EB_PUBLIC eb_status_t eb_sdb_find_by_identity(eb_device_t device, uint64_t vendor_id, uint32_t device_id, struct sdb_device* output, int* devices);


/* Similar to eb_sdb_find_by_identity, but the root of the SDB tree to be searched can be specified.
 * eb_sdb_find_by_identity now supports finding crossbars, use it to find special CBs and use them as
 * root node for eb_sdb_find_by_identity_at. 
 */
EB_PUBLIC eb_status_t eb_sdb_find_by_identity_at(eb_device_t device, const struct sdb_bridge* bridge, uint64_t vendor_id, uint32_t device_id, struct sdb_device* output, int* devices);

#ifdef __cplusplus
}

#include <vector>
#include <ostream>

/****************************************************************************/
/*                                 C++ API                                  */
/****************************************************************************/

namespace etherbone {

/* Copy the types into the namespace */
typedef eb_address_t address_t;
typedef eb_data_t data_t;
typedef eb_format_t format_t;
typedef eb_status_t status_t;
typedef eb_width_t width_t;
typedef eb_descriptor_t descriptor_t;

class Socket;
class Device;
class Cycle;
class Operation;

#if ETHERBONE_THROWS
struct exception_t {
  const char* method;
  status_t    status;
  exception_t(const char* m = "", status_t s = EB_OK) : method(m), status(s) { }
};
inline std::ostream& operator << (std::ostream& o, const exception_t& e) {
  return o << e.method << ": " << eb_status(e.status);
}
#define EB_STATUS_OR_VOID_T void
#define EB_RETURN_OR_THROW(m, x) do { status_t result = x; if (result != EB_OK) { throw exception_t(m, result); } } while (0)
#else
#define EB_STATUS_OR_VOID_T status_t
#define EB_RETURN_OR_THROW(m, x) return x
#endif

class Handler {
  public:
    EB_PUBLIC virtual ~Handler();

    virtual status_t read (address_t address, width_t width, data_t* data) = 0;
    virtual status_t write(address_t address, width_t width, data_t  data) = 0;
};

class Socket {
  public:
    Socket();
    
    EB_STATUS_OR_VOID_T open(const char* port = 0, width_t width = EB_DATAX|EB_ADDRX);
    EB_STATUS_OR_VOID_T close();
    
    EB_STATUS_OR_VOID_T passive(const char* address);
    
    /* attach/detach a virtual device */
    EB_STATUS_OR_VOID_T attach(const struct sdb_device* device, Handler* handler);
    EB_STATUS_OR_VOID_T detach(const struct sdb_device* device);
    
    int run(int timeout_us = -1);
    
    /* These can be used to implement your own 'block': */
    uint32_t timeout() const;
    void descriptors(eb_user_data_t user, eb_descriptor_callback_t list) const;
    int check(uint32_t now, eb_user_data_t user, eb_descriptor_callback_t ready);
    
  protected:
    Socket(eb_socket_t sock);
    eb_socket_t socket;
  
  friend class Device;
};

class Device {
  public:
    Device();
    
    EB_STATUS_OR_VOID_T open(Socket socket, const char* address, width_t width = EB_ADDRX|EB_DATAX, int attempts = 5);
    EB_STATUS_OR_VOID_T close();
    
    const Socket socket() const;
    Socket socket();
    
    width_t width() const;
    EB_STATUS_OR_VOID_T get_irq_range(eb_data_t* irq_start_high, eb_data_t* irq_start_low, eb_data_t* irq_end_high, eb_data_t* irq_end_low);
    
    template <typename T>
    EB_STATUS_OR_VOID_T sdb_scan_bus (const struct sdb_bridge* bridge, T* user, sdb_callback_t);
    template <typename T>
    EB_STATUS_OR_VOID_T sdb_scan_root(T* user, sdb_callback_t);
    
    EB_STATUS_OR_VOID_T sdb_find_by_address(eb_address_t address, struct sdb_device* output);
    EB_STATUS_OR_VOID_T sdb_find_by_identity(uint64_t vendor_id, uint32_t device_id, std::vector<struct sdb_device>& output);
    
    template <typename T>
    EB_STATUS_OR_VOID_T read(eb_address_t address, eb_format_t format, eb_data_t* data, T* user, eb_callback_t cb);
    EB_STATUS_OR_VOID_T read(eb_address_t address, eb_format_t format, eb_data_t* data);
    
    template <typename T>
    EB_STATUS_OR_VOID_T write(eb_address_t address, eb_format_t format, eb_data_t data, T* user, eb_callback_t cb);
    EB_STATUS_OR_VOID_T write(eb_address_t address, eb_format_t format, eb_data_t data);
    
  protected:
    Device(eb_device_t device);
    eb_device_t device;
  
  friend class Cycle;
  template <typename T, void (T::*cb)(Device, Operation, status_t)>
  friend void wrap_member_callback(eb_user_data_t object, eb_device_t dev, eb_operation_t op, eb_status_t status);
  template <typename T, void (*cb)(T*, Device, Operation, status_t)>
  friend void wrap_function_callback(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status);
  template <typename T, void (T::*cb)(Device, const struct sdb_table*, status_t)>
  friend void sdb_wrap_member_callback(eb_user_data_t user, eb_device_t dev, const struct sdb_table* sdb, eb_status_t status);
  template <typename T, void (*cb)(T* user, Device, const struct sdb_table*, status_t)>
  friend void sdb_wrap_function_callback(eb_user_data_t user, eb_device_t dev, const struct sdb_table* sdb, eb_status_t status);
};

class Cycle {
  public:
    Cycle();
    
    // Start a cycle on the target device.
    template <typename T>
    EB_STATUS_OR_VOID_T open(Device device, T* user, eb_callback_t);
    EB_STATUS_OR_VOID_T open(Device device);
    
    void abort();
    EB_STATUS_OR_VOID_T close();
    EB_STATUS_OR_VOID_T close_silently();
    
    void read (address_t address, format_t format = EB_DATAX, data_t* data = 0);
    void write(address_t address, format_t format, data_t  data);
    
    void read_config (address_t address, format_t format = EB_DATAX, data_t* data = 0);
    void write_config(address_t address, format_t format, data_t  data);
    
    const Device device() const;
    Device device();
    
  protected:
    Cycle(eb_cycle_t cycle);
    eb_cycle_t cycle;
};

class Operation {
  public:
    bool is_null  () const;
    
    /* Only call these if is_null is false */
    bool is_read  () const;
    bool is_config() const;
    bool had_error() const;
    
    address_t address() const;
    data_t    data   () const;
    format_t  format () const;
    
    const Operation next() const;
    Operation next();
    
  protected:
    Operation(eb_operation_t op);
    
    eb_operation_t operation;

  template <typename T, void (T::*cb)(Device, Operation, status_t)>
  friend void wrap_member_callback(eb_user_data_t object, eb_device_t dev, eb_operation_t op, eb_status_t status);
  template <typename T, void (*cb)(T*, Device, Operation, status_t)>
  friend void wrap_function_callback(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status);
};

/* Convenience templates to convert member functions into callback type */
template <typename T, void (T::*cb)(Device, Operation, status_t)>
void wrap_member_callback(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status) {
  return (reinterpret_cast<T*>(user)->*cb)(Device(dev), Operation(op), status);
}
template <typename T, void (*cb)(T* user, Device, Operation, status_t)>
void wrap_function_callback(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status) {
  return (*cb)(reinterpret_cast<T*>(user), Device(dev), Operation(op), status);
}

template <typename T, void (T::*cb)(Device, const struct sdb_table*, status_t)>
void sdb_wrap_member_callback(eb_user_data_t user, eb_device_t dev, const struct sdb_table* sdb, eb_status_t status) {
  return (reinterpret_cast<T*>(user)->*cb)(Device(dev), sdb, status);
}

template <typename T, void (*cb)(T* user, Device, const struct sdb_table*, status_t)>
void sdb_wrap_function_callback(eb_user_data_t user, eb_device_t dev, const struct sdb_table* sdb, eb_status_t status) {
  return (*cb)(reinterpret_cast<T*>(user), Device(dev), sdb, status);
}

/****************************************************************************/
/*                            C++ Implementation                            */
/****************************************************************************/

/* Proxy functions needed by C++ -- ignore these */
EB_PUBLIC eb_status_t eb_proxy_read_handler(eb_user_data_t data, eb_address_t address, eb_width_t width, eb_data_t* ptr);
EB_PUBLIC eb_status_t eb_proxy_write_handler(eb_user_data_t data, eb_address_t address, eb_width_t width, eb_data_t value);

inline Socket::Socket(eb_socket_t sock)
 : socket(sock) { 
}

inline Socket::Socket()
 : socket(EB_NULL) {
}

inline EB_STATUS_OR_VOID_T Socket::open(const char* port, width_t width) {
  EB_RETURN_OR_THROW("Socket::open", eb_socket_open(EB_ABI_CODE, port, width, &socket));
}

inline EB_STATUS_OR_VOID_T Socket::close() {
  status_t out = eb_socket_close(socket);
  if (out == EB_OK) socket = EB_NULL;
  EB_RETURN_OR_THROW("Socket::close", out);
}

inline EB_STATUS_OR_VOID_T Socket::passive(const char* address) {
  EB_RETURN_OR_THROW("Socket::passive", eb_socket_passive(socket, address));
}

inline EB_STATUS_OR_VOID_T Socket::attach(const struct sdb_device* device, Handler* handler) {
  struct eb_handler h;
  h.device = device;
  h.data = handler;
  h.read  = &eb_proxy_read_handler;
  h.write = &eb_proxy_write_handler;
  EB_RETURN_OR_THROW("Socket::attach", eb_socket_attach(socket, &h));
}

inline EB_STATUS_OR_VOID_T Socket::detach(const struct sdb_device* device) {
  EB_RETURN_OR_THROW("Socket::detach", eb_socket_detach(socket, device));
}

inline int Socket::run(int timeout_us) {
  return eb_socket_run(socket, timeout_us);
}

inline uint32_t Socket::timeout() const {
  return eb_socket_timeout(socket);
}

inline void Socket::descriptors(eb_user_data_t user, eb_descriptor_callback_t list) const {
  return eb_socket_descriptors(socket, user, list);
}

inline int Socket::check(uint32_t now, eb_user_data_t user, eb_descriptor_callback_t ready) {
  return eb_socket_check(socket, now, user, ready);
}

inline Device::Device(eb_device_t dev)
 : device(dev) {
}

inline Device::Device()
 : device(EB_NULL) { 
}
    
inline EB_STATUS_OR_VOID_T Device::open(Socket socket, const char* address, width_t width, int attempts) {
  EB_RETURN_OR_THROW("Device::open", eb_device_open(socket.socket, address, width, attempts, &device));
}
    
inline EB_STATUS_OR_VOID_T Device::close() {
  status_t out = eb_device_close(device);
  if (out == EB_OK) device = EB_NULL;
  EB_RETURN_OR_THROW("Device::close", out);
}

inline const Socket Device::socket() const {
  return Socket(eb_device_socket(device));
}

inline Socket Device::socket() {
  return Socket(eb_device_socket(device));
}

inline width_t Device::width() const {
  return eb_device_width(device);
}

inline EB_STATUS_OR_VOID_T Device::get_irq_range(eb_data_t* irq_start_high, eb_data_t* irq_start_low, eb_data_t* irq_end_high, eb_data_t* irq_end_low) {
  EB_RETURN_OR_THROW ("Device::get_irq_range", eb_device_get_irq_range(device, irq_start_high, irq_start_low, irq_end_high, irq_end_low));
}

template <typename T>
inline EB_STATUS_OR_VOID_T Device::sdb_scan_bus(const struct sdb_bridge* bridge, T* user, sdb_callback_t cb) {
  EB_RETURN_OR_THROW("Device::sdb_scan_bus", eb_sdb_scan_bus(device, bridge, user, cb));
}

template <typename T>
inline EB_STATUS_OR_VOID_T Device::sdb_scan_root(T* user, sdb_callback_t cb) {
  EB_RETURN_OR_THROW("Device::sdb_scan_root", eb_sdb_scan_root(device, user, cb));
}

inline EB_STATUS_OR_VOID_T Device::sdb_find_by_address(eb_address_t address, struct sdb_device* output) {
  EB_RETURN_OR_THROW("Device::sdb_find_by_address", eb_sdb_find_by_address(device, address, output));
}

template <typename T>
inline EB_STATUS_OR_VOID_T Device::read(eb_address_t address, eb_format_t format, eb_data_t* data, T* user, eb_callback_t cb) {
  EB_RETURN_OR_THROW("Device::read", eb_device_read(device, address, format, data, user, cb));
}

inline EB_STATUS_OR_VOID_T Device::read(eb_address_t address, eb_format_t format, eb_data_t* data) {
  EB_RETURN_OR_THROW("Device::read", eb_device_read(device, address, format, data, 0, eb_block));
}

template <typename T>
inline EB_STATUS_OR_VOID_T Device::write(eb_address_t address, eb_format_t format, eb_data_t data, T* user, eb_callback_t cb) {
  EB_RETURN_OR_THROW("Device::write", eb_device_write(device, address, format, data, user, cb));
}

inline EB_STATUS_OR_VOID_T Device::write(eb_address_t address, eb_format_t format, eb_data_t data) {
  EB_RETURN_OR_THROW("Device::write", eb_device_write(device, address, format, data, 0, eb_block));
}

inline Cycle::Cycle()
 : cycle(EB_NULL) {
}

template <typename T>
inline EB_STATUS_OR_VOID_T Cycle::open(Device device, T* user, eb_callback_t cb) {
  EB_RETURN_OR_THROW("Cycle::open", eb_cycle_open(device.device, user, cb, &cycle));
}

inline EB_STATUS_OR_VOID_T Cycle::open(Device device) {
  EB_RETURN_OR_THROW("Cycle::open", eb_cycle_open(device.device, 0, eb_block, &cycle));
}

inline void Cycle::abort() {
  eb_cycle_abort(cycle);
  cycle = EB_NULL;
}

inline EB_STATUS_OR_VOID_T Cycle::close() {
  status_t status;
  status = eb_cycle_close(cycle);
  cycle = EB_NULL;
  EB_RETURN_OR_THROW("Cycle::close", status);
}

inline EB_STATUS_OR_VOID_T Cycle::close_silently() {
  status_t status;
  status = eb_cycle_close_silently(cycle);
  cycle = EB_NULL;
  EB_RETURN_OR_THROW("Cycle::close_silently", status);
}

inline void Cycle::read(address_t address, format_t format, data_t* data) {
  eb_cycle_read(cycle, address, format, data);
}

inline void Cycle::write(address_t address, format_t format, data_t data) {
  eb_cycle_write(cycle, address, format, data);
}

inline void Cycle::read_config(address_t address, format_t format, data_t* data) {
  eb_cycle_read_config(cycle, address, format, data);
}

inline void Cycle::write_config(address_t address, format_t format, data_t data) {
  eb_cycle_write_config(cycle, address, format, data);
}

inline const Device Cycle::device() const {
  return Device(eb_cycle_device(cycle));
}

inline Device Cycle::device() {
  return Device(eb_cycle_device(cycle));
}

inline Operation::Operation(eb_operation_t op)
 : operation(op) {
}

inline bool Operation::is_null() const {
  return operation == EB_NULL;
}

inline bool Operation::is_read() const {
  return eb_operation_is_read(operation);
}

inline bool Operation::is_config() const {
  return eb_operation_is_config(operation);
}

inline bool Operation::had_error() const {
  return eb_operation_had_error(operation);
}

inline address_t Operation::address() const {
  return eb_operation_address(operation);
}

inline format_t Operation::format() const {
  return eb_operation_format(operation);
}

inline data_t Operation::data() const {
  return eb_operation_data(operation);
}

inline Operation Operation::next() {
  return Operation(eb_operation_next(operation));
}

inline const Operation Operation::next() const {
  return Operation(eb_operation_next(operation));
}

}

#endif

#endif
