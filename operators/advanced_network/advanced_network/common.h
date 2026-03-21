/*
 * SPDX-FileCopyrightText: Copyright (c) 2023-2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <vector>
#include <string>
#include <unordered_set>
#include <memory>
#include <optional>
#include <tuple>
#include <stdint.h>
#include "advanced_network/types.h"

namespace holoscan::advanced_network {

// this part is purely optional, just a helper for the user
BurstParams* create_burst_params();
BurstParams* create_tx_burst_params();

enum class ErrorGlobalStats {
  OUT_OF_RX_BUFFERS = 0,
  RX_QUEUE_FULL = 1,
  METADATA_BUF_DEPLETED = 2,

  SENTINEL = 3,
};

static constexpr uint32_t DEFAULT_TX_META_BUFFERS = 1UL << 8;
static constexpr uint32_t DEFAULT_RX_META_BUFFERS = 1UL << 8;
namespace detail {
inline Direction DirectionStringToType(const std::string& dir) {
  if (dir == "rx") {
    return Direction::RX;
  } else if (dir == "tx") {
    return Direction::TX;
  }

  return Direction::TX_RX;
}
};  // namespace detail

/**
 * @brief Determine which directions are enabled
 *
 * @param dir Direction from config. Either "rx", "tx", or "tx/rx"
 * @return int Number of directions enabled
 */
inline int EnabledDirections(const std::string& dir) {
  if (dir == "rx" || dir == "tx") { return 1; }

  return 0;
}

/**
 * @brief Initialize the backend manager and any other resources needed
 *
 * @param config YML Configuration structure (e.g. AdvNetConfigYaml)
 * @return AdvNetStatus indicating status. Valid values are:
 *    SUCCESS: Initialization successful
 *    INVALID_CONFIG: Invalid configuration
 *    INTERNAL_ERROR: Internal error
 */
Status adv_net_init(NetworkConfig& config);

/**
 * @brief Returns a manager type
 *
 * @return Manager type
 */
ManagerType get_manager_type();

/**
 * @brief Returns a manager type
 *
 * @param config YML Configuration structure (e.g. NetworkConfig)
 * @return Manager type
 */
ManagerType get_manager_type(const NetworkConfig& config);

/**
 * @brief Returns a raw packet pointer from a pointer in BurstParams
 *
 * The BurstParams structure contains pointers to opaque packets which are not accessible
 * directly by the user. This function fetches the CPU packet pointer at index idx
 * from the burst.
 *
 * @param burst Burst structure containing packets
 * @param seg Segment of packet
 * @param idx Index of packet
 * @return Pointer to packet data
 */
void* get_segment_packet_ptr(BurstParams* burst, int seg, int idx);

/**
 * @brief Returns a raw packet pointer from a pointer in BurstParams
 *
 * The BurstParams structure contains pointers to opaque packets which are not accessible
 * directly by the user. This function fetches the GPU packet pointer at index idx
 * from the burst.
 *
 * @param burst Burst structure containing packets
 * @param idx Index of packet
 * @return Pointer to packet data
 */
void* get_packet_ptr(BurstParams* burst, int idx);

/**
 * @brief Get packet length of a segment of a packet
 *
 * @param burst Burst structure containing packets
 * @param seg Segment of packet
 * @param idx Index of packet
 * @return uint16_t Length of packet
 */
uint32_t get_segment_packet_length(BurstParams* burst, int seg, int idx);

/**
 * @brief Get packet length of an entire packet
 *
 * @param burst Burst structure containing packets
 * @param idx Index of packet
 * @return uint32_t Length of packet
 */
uint32_t get_packet_length(BurstParams* burst, int idx);

/**
 * @brief Get flow ID of a packet
 *
 * Retrieves the flow ID of a packet, or 0 if no flow was matched. The flow ID should match
 * the flow ID in the flow rule for the advanced_network config.
 *
 * @param burst Burst structure containing packets
 * @param idx Index of packet
 * @return uint16_t Flow ID
 */
uint16_t get_packet_flow_id(BurstParams* burst, int idx);

/**
 * @brief Populate a TX packet burst buffer
 *
 * Populates a transmit packet burst buffer with allocated packets. The user can take these
 * allocated packets and fill with the desired data/headers.
 *
 * @param burst Burst structure to populate
 * @return Status indicating status. Valid values are:
 *    SUCCESS: Packets allocated
 *    NULL_PTR: Burst or packet pools uninitialized
 *    NO_FREE_BURST_BUFFERS: No burst buffers to allocate
 *    NO_FREE_CPU_PACKET_BUFFERS: Not enough CPU packet buffers available
 */
Status get_tx_packet_burst(BurstParams* burst);

/**
 * @brief Set IPv4 header in packet
 *
 * @param burst Burst structure to populate
 * @param idx Index of packet
 * @param dst_addr Ethernet destination address
 * @return Status indicating status. Valid values are:
 *    SUCCESS: Packet populated successfully
 */
Status set_eth_header(BurstParams* burst, int idx, char* dst_addr);

/**
 * @brief Set IPv4 header in packet
 *
 * @param burst Burst structure to populate
 * @param idx Index of packet
 * @param ip_len Length of packet after IPv4 header
 * @param proto L4 protocol
 * @param src_host Source host in host byte order
 * @param dst_host Destination host in host byte order
 * @return Status indicating status. Valid values are:
 *    SUCCESS: Packet populated successfully
 */
Status set_ipv4_header(BurstParams* burst, int idx, int ip_len, uint8_t proto,
                       unsigned int src_host, unsigned int dst_host);

/**
 * @brief Set UDP header in packet
 *
 * @param burst Burst structure to populate
 * @param idx Index of packet
 * @param udp_len Length of packet after UDP header
 * @param src_port Source port
 * @param dst_port Destination port
 * @return Status indicating status. Valid values are:
 *    SUCCESS: Packet populated successfully
 */
Status set_udp_header(BurstParams* burst, int idx, int udp_len, uint16_t src_port,
                      uint16_t dst_port);

/**
 * @brief Set UDP payload in packet
 *
 * @param burst Burst structure to populate
 * @param idx Index of packet
 * @param data Payload data after UDP header
 * @param len Length of payload
 * @return Status indicating status. Valid values are:
 *    SUCCESS: Packet populated successfully
 */
Status set_udp_payload(BurstParams* burst, int idx, void* data, int len);

/**
 * @brief Test if a TX burst is available
 *
 * Checks whether a TX burst for a given size can be allocated. This is useful for an
 * application to throttle its transmissions if the NIC is not keeping up with the desired rate.
 * Rather than returning an error, the user can use this function to loop or return later
 * to try again.
 *
 * @param burst Info about burst of packets
 * @return true Burst is available
 * @return false Burst is not available
 */
bool is_tx_burst_available(BurstParams* burst);

/**
 * @brief Free all packets and burst from one segment
 *
 * Frees every allocated packets in the burst and the burst metadata for one segment.
 * After this call completes the segment's pointers are no longer valid.
 *
 * @param burst Burst to free
 */
void free_segment_packets_and_burst(BurstParams* burst, int seg);

/**
 * @brief Free all packets and an RX burst
 *
 * Frees all packets in a burst of packets and the associated burst buffer
 *
 * @param burst Burst structure containing packet lists
 */
void free_all_packets_and_burst_rx(BurstParams* burst);

/**
 * @brief Free all packets and a TX burst
 *
 * Frees all packets in a burst of packets and the associated burst buffer
 *
 * @param burst Burst structure containing packet lists
 */
void free_all_packets_and_burst_tx(BurstParams* burst);

/**
 * @brief Set packet lengths in metadata
 *
 * Sets metadata packet lengths. This is needed in addition to L3+L4 lengths for hardware
 *
 * @param burst Burst structure containing packet lists
 * @param idx Index of packet
 * @param lens Lengths of each segment
 * @return Status indicating status. Valid values are:
 *    SUCCESS: Packet populated successfully
 */
Status set_packet_lengths(BurstParams* burst, int idx, const std::initializer_list<int>& lens);

/**
 * @brief Set packet TX time
 *
 * Sets the transmit time (in PTP time) to transmit the packet. Every packet transmitted
 * after this one in the same queue will be transmitted no earlier than the time listed
 * in the function call. This feature is only available on ConnectX-7 or BlueField 3 and
 * higher cards.
 *
 * @param burst Burst structure containing packet lists
 * @param idx Index of packet
 * @param time PTP time to transmit
 * @return Status indicating status. Valid values are:
 *    SUCCESS: Time set successfully
 */
Status set_packet_tx_time(BurstParams* burst, int idx, uint64_t time);

uint64_t get_burst_tot_byte(BurstParams* burst);

/**
 * @brief Frees all segments of a single packet
 *
 * @param burst Burst structure containing packet lists
 * @param idx Index of packet
 */
void free_packet(BurstParams* burst, int idx);

/**
 * @brief Frees a single segment from a single packet
 *
 * @param burst Burst structure containing packet lists
 * @param seg Segment of packet in scatter list
 * @param idx Index of packet
 */
void free_packet_segment(BurstParams* burst, int seg, int idx);

/**
 * @brief Free all packets for a single segment in a burst
 *
 * Frees all packets in a single segment in a burst of packets.
 *
 * @param burst Burst structure containing packet lists
 */
void free_all_segment_packets(BurstParams* burst, int seg);

/**
 * @brief Free a receive burst
 *
 * Frees the buffer containing a receive burst buffer. This function does not free packets;
 * packets must be freed prior to calling this.
 *
 * @param burst
 */
void free_rx_burst(BurstParams* burst);

/**
 * @brief Free a transmit burst buffer
 *
 * Frees the buffer containing a transmit burst buffer. This function does not free packets;
 * packets must be freed prior to calling this.
 *
 * @param burst Burst structure to free
 */
void free_tx_burst(BurstParams* burst);

/**
 * @brief Free a receive TX meta buffer
 *
 * Frees the buffer containing a receive TX meta buffer. This function does not free packets;
 * packets must be freed prior to calling this.
 *
 * @param burst Burst structure to free
 */
void free_tx_metadata(BurstParams* burst);

/**
 * @brief Free a receive RX meta buffer
 *
 * Frees the buffer containing a receive RX meta buffer. This function does not free packets;
 * packets must be freed prior to calling this.
 *
 * @param burst Burst structure to free
 */
void free_rx_metadata(BurstParams* burst);

/**
 * @brief Get the number of packets in a burst
 *
 * @param burst Burst structure with packets
 */
int64_t get_num_packets(BurstParams* burst);

/**
 * @brief Get the queue ID of a burst
 *
 * @param burst Burst structure with packets
 */
int64_t get_q_id(BurstParams* burst);

/**
 * @brief Get mac address of an interface
 *
 * @param port Port number of interface
 * @param mac MAC address of interface
 *
 * @returns Status::SUCCESS on success
 */
Status get_mac_addr(int port, char* mac);

/**
 * @brief Drop all traffic on a port
 *
 * Creates a high-priority flow rule that drops all incoming traffic on the specified port.
 * This acts as a "kill switch" for traffic. Use allow_all_traffic() to remove the drop rule.
 *
 * @param port Port number of interface
 *
 * @returns Status::SUCCESS on success, error status on failure
 */
Status drop_all_traffic(int port);

/**
 * @brief Allow all traffic on a port
 *
 * Removes a previously installed drop rule created by drop_all_traffic(), restoring
 * normal traffic flow on the port.
 *
 * @param port Port number of interface
 *
 * @returns Status::SUCCESS on success, error status on failure
 */
Status allow_all_traffic(int port);

/**
 * @brief Get port number from interface name
 *
 * @param key PCIe address or config name of the interface to look up
 *
 * @returns Port number or -1 for not found
 */
int get_port_id(const std::string& key);

/**
 * @brief Set the number of packets in a burst
 *
 * @param burst Burst structure
 * @param num Number of packets
 */
void set_num_packets(BurstParams* burst, int64_t num);

/**
 * @brief Send a TX burst
 *
 * @param burst Burst structure
 * @return Status indicating status. Valid values are:
 *    SUCCESS: Burst sent successfully
 */
Status send_tx_burst(BurstParams* burst);

/**
 * @brief Get a RX burst
 *
 * @param burst Burst structure
 * @param port Port ID of interface
 * @param q Queue ID of interface
 * @return Status indicating status. Valid values are:
 *    SUCCESS: Burst received successfully
 *    NULL_PTR: No bursts ready to receive
 */
Status get_rx_burst(BurstParams** burst, int port, int q);

/**
 * @brief Get a RX burst from any queue on a specific port
 *
 * @param burst Burst structure
 * @param port Port ID of interface
 * @return Status indicating status. Valid values are:
 *    SUCCESS: Burst received successfully
 *    NULL_PTR: No bursts ready to receive on any queue for this port
 */
Status get_rx_burst(BurstParams** burst, int port);

/**
 * @brief Get a RX burst from any queue on any port
 *
 * @param burst Burst structure
 * @return Status indicating status. Valid values are:
 *    SUCCESS: Burst received successfully
 *    NULL_PTR: No bursts ready to receive on any queue on any port
 */
Status get_rx_burst(BurstParams** burst);

/**
 * @brief Get a RX burst
 *
 * @param burst Burst structure
 * @param conn_id Connection ID representing a unique ID for a client/server connection
 * @param server True if server, false if client. Used to determine which ring to dequeue from.
 */
Status get_rx_burst(BurstParams** burst, uintptr_t conn_id, bool server);

/**
 * @brief Set the header fields in a burst
 *
 * @param burst Burst structure
 * @param port Port ID of interface
 * @param q Queue ID of interface
 * @param num Number of packets
 * @param segs Number of segments
 */
void set_header(BurstParams* burst, uint16_t port, uint16_t q, int64_t num, int segs);

/**
 * @brief First MAC address string to char buffer
 *
 * @param dst Destination buffer
 * @param addr MAC address as string in format xx:xx:xx:xx:xx:xx
 */
void format_eth_addr(char* dst, std::string addr);

/**
 * @brief Shut down the advanced_network and do any cleanup necessary. Freeing memory is done
 * in the manager's destructor.
 *
 */
void shutdown();

/**
 * @brief Print port statistics
 *
 */
void print_stats();

/**
 * @brief Get the number of RX queues. May be overridden by the manager if the number of queues
 * differs from what is defined in the config.
 *
 * @param port_id Port ID of interface
 * @return uint16_t Number of RX queues
 */
uint16_t get_num_rx_queues(int port_id);

/**
 * @brief Flush all packets from a specific port/queue
 *
 * Drains and discards all packets currently in the specified queue on the specified port.
 * This is useful for clearing stale packets from a queue before starting operations.
 *
 * @param port Port number of interface
 * @param queue Queue ID on the port
 */
void flush_port_queue(int port, int queue);

// RDMA functions
Status rdma_connect_to_server(const std::string& server_addr, uint16_t server_port,
                              uintptr_t* conn_id);
Status rdma_connect_to_server(const std::string& server_addr, uint16_t server_port,
                              const std::string& src_addr, uintptr_t* conn_id);
Status rdma_get_port_queue(uintptr_t conn_id, uint16_t* port, uint16_t* queue);
Status rdma_get_server_conn_id(const std::string& server_addr, uint16_t server_port,
                               uintptr_t* conn_id);
Status rdma_set_header(BurstParams* burst, RDMAOpCode op_code, uintptr_t conn_id, bool is_server,
                       int num_pkts, uint64_t wr_id, const std::string& local_mr_name);
RDMAOpCode rdma_get_opcode(BurstParams* burst);

};  // namespace holoscan::advanced_network
