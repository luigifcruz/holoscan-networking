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

#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <arpa/inet.h>

#include "advanced_network/manager.h"
#include "advanced_network/common.h"
#if ANO_MGR_DPDK
#include <rte_mbuf.h>
#include <rte_memcpy.h>
#include <rte_ethdev.h>
#endif

#define ASSERT_ANO_MGR_INITIALIZED() \
  assert(g_ano_mgr != nullptr && "Advanced Network Manager is not initialized")
namespace holoscan::advanced_network {

// Declare a static global variable for the manager
static Manager* g_ano_mgr = nullptr;

const std::unordered_map<LogLevel::Level, std::string> LogLevel::level_to_string_map = {
    {TRACE, "trace"},
    {DEBUG, "debug"},
    {INFO, "info"},
    {WARN, "warn"},
    {ERROR, "error"},
    {CRITICAL, "critical"},
    {OFF, "off"},
};

const std::unordered_map<std::string, LogLevel::Level> LogLevel::string_to_level_map = {
    {"trace", TRACE},
    {"debug", DEBUG},
    {"info", INFO},
    {"warn", WARN},
    {"error", ERROR},
    {"critical", CRITICAL},
    {"off", OFF},
};

[[deprecated("Use create_tx_burst_params() instead")]] BurstParams* create_burst_params() {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->create_tx_burst_params();
}

BurstParams* create_tx_burst_params() {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->create_tx_burst_params();
}

void initialize_manager(Manager* manager) {
  g_ano_mgr = manager;
}

Manager* get_active_manager() {
  return g_ano_mgr;
}

ManagerType get_manager_type() {
  return ManagerFactory::get_manager_type();
}

ManagerType get_manager_type(const NetworkConfig& config) {
  return ManagerFactory::get_manager_type(config);
}

void free_packet(BurstParams* burst, int pkt) {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->free_packet(burst, pkt);
}

void free_packet_segment(BurstParams* burst, int seg, int pkt) {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->free_packet_segment(burst, seg, pkt);
}

uint32_t get_packet_length(BurstParams* burst, int idx) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_packet_length(burst, idx);
}

uint16_t get_packet_flow_id(BurstParams* burst, int idx) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_packet_flow_id(burst, idx);
}

uint64_t get_burst_tot_byte(BurstParams* burst) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_burst_tot_byte(burst);
}

uint32_t get_segment_packet_length(BurstParams* burst, int seg, int idx) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_segment_packet_length(burst, seg, idx);
}

void free_all_segment_packets(BurstParams* burst, int seg) {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->free_all_segment_packets(burst, seg);
}

void free_all_burst_packets(BurstParams* burst) {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->free_all_packets(burst);
}

void free_all_packets_and_burst_rx(BurstParams* burst) {
  free_all_burst_packets(burst);
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->free_rx_burst(burst);
}

void free_all_packets_and_burst_tx(BurstParams* burst) {
  free_all_burst_packets(burst);
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->free_tx_burst(burst);
}

void free_segment_packets_and_burst(BurstParams* burst, int seg) {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->free_all_segment_packets(burst, seg);
  g_ano_mgr->free_rx_burst(burst);
}

void format_eth_addr(char* dst, std::string addr) {
  std::istringstream iss(addr);
  std::string byteString;

  uint8_t byte_cnt = 0;
  while (std::getline(iss, byteString, ':')) {
    if (byteString.length() == 2) {
      uint16_t byte = std::stoi(byteString, nullptr, 16);
      dst[byte_cnt++] = static_cast<char>(byte);
    } else {
      HOLOSCAN_LOG_ERROR("Invalid MAC address format: {}", addr);
      dst[0] = 0x00;
    }
  }
}

Status get_mac_addr(int port, char* mac) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_mac_addr(port, mac);
}

Status drop_all_traffic(int port) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->drop_all_traffic(port);
}

Status allow_all_traffic(int port) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->allow_all_traffic(port);
}

bool is_tx_burst_available(BurstParams* burst) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->is_tx_burst_available(burst);
}

int get_port_id(const std::string& key) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_port_id(key);
}

Status get_tx_packet_burst(BurstParams* burst) {
  ASSERT_ANO_MGR_INITIALIZED();
  if (!g_ano_mgr->is_tx_burst_available(burst)) return Status::NO_FREE_BURST_BUFFERS;
  return g_ano_mgr->get_tx_packet_burst(burst);
}

Status set_eth_header(BurstParams* burst, int idx, char* dst_addr) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->set_eth_header(burst, idx, dst_addr);
}

Status set_ipv4_header(BurstParams* burst, int idx, int ip_len, uint8_t proto,
                       unsigned int src_host, unsigned int dst_host) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->set_ipv4_header(burst, idx, ip_len, proto, src_host, dst_host);
}

Status set_udp_header(BurstParams* burst, int idx, int udp_len, uint16_t src_port,
                      uint16_t dst_port) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->set_udp_header(burst, idx, udp_len, src_port, dst_port);
}

Status set_udp_payload(BurstParams* burst, int idx, void* data, int len) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->set_udp_payload(burst, idx, data, len);
}

Status set_packet_lengths(BurstParams* burst, int idx, const std::initializer_list<int>& lens) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->set_packet_lengths(burst, idx, lens);
}

Status set_packet_tx_time(BurstParams* burst, int idx, uint64_t time) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->set_packet_tx_time(burst, idx, time);
}

int64_t get_num_packets(BurstParams* burst) {
  return burst->hdr.hdr.num_pkts;
}

int64_t get_q_id(BurstParams* burst) {
  assert(burst != nullptr && "burst is null");
  return burst->hdr.hdr.q_id;
}

void set_num_packets(BurstParams* burst, int64_t num) {
  assert(burst != nullptr && "burst is null");
  burst->hdr.hdr.num_pkts = num;
}

void set_header(BurstParams* burst, uint16_t port, uint16_t q, int64_t num, int segs) {
  assert(burst != nullptr && "burst is null");
  burst->hdr.hdr.num_pkts = num;
  burst->hdr.hdr.port_id = port;
  burst->hdr.hdr.q_id = q;
  burst->hdr.hdr.num_segs = segs;
}

void free_tx_burst(BurstParams* burst) {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->free_tx_burst(burst);
}

void free_tx_metadata(BurstParams* burst) {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->free_tx_metadata(burst);
}

void free_rx_burst(BurstParams* burst) {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->free_rx_burst(burst);
}

void free_rx_metadata(BurstParams* burst) {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->free_rx_metadata(burst);
}

void* get_segment_packet_ptr(BurstParams* burst, int seg, int idx) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_segment_packet_ptr(burst, seg, idx);
}

void* get_packet_ptr(BurstParams* burst, int idx) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_packet_ptr(burst, idx);
}

void shutdown() {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->shutdown();
}

Status send_tx_burst(BurstParams* burst) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->send_tx_burst(burst);
}

Status get_rx_burst(BurstParams** burst, int port, int q) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_rx_burst(burst, port, q);
}

Status get_rx_burst(BurstParams** burst, int port) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_rx_burst(burst, port);
}

Status get_rx_burst(BurstParams** burst) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_rx_burst(burst);
}

Status get_rx_burst(BurstParams** burst, uintptr_t conn_id, bool server) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_rx_burst(burst, conn_id, server);
}

uint16_t get_num_rx_queues(int port_id) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->get_num_rx_queues(port_id);
}

void flush_port_queue(int port, int queue) {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->flush_port_queue(port, queue);
}

void print_stats() {
  ASSERT_ANO_MGR_INITIALIZED();
  g_ano_mgr->print_stats();
}

Status adv_net_init(NetworkConfig& config) {
  ManagerFactory::set_manager_type(config.common_.manager_type);

  auto mgr = &(ManagerFactory::get_active_manager());

  if (!mgr->set_config_and_initialize(config)) { return Status::INTERNAL_ERROR; }

  for (const auto& intf : config.ifs_) {
    auto port = mgr->get_port_id(intf.address_);
    if (port < 0) {
      HOLOSCAN_LOG_ERROR("Failed to get port from name {}", intf.address_);
      return Status::INVALID_PARAMETER;
    }
  }

  return Status::SUCCESS;
}

// RDMA Functions
Status rdma_connect_to_server(const std::string& server_addr, uint16_t server_port,
                              uintptr_t* conn_id) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->rdma_connect_to_server(server_addr, server_port, conn_id);
}

Status rdma_connect_to_server(const std::string& server_addr, uint16_t server_port,
                              const std::string& src_addr, uintptr_t* conn_id) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->rdma_connect_to_server(server_addr, server_port, src_addr, conn_id);
}

Status rdma_get_port_queue(uintptr_t conn_id, uint16_t* port, uint16_t* queue) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->rdma_get_port_queue(conn_id, port, queue);
}

Status rdma_get_server_conn_id(const std::string& server_addr, uint16_t server_port,
                               uintptr_t* conn_id) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->rdma_get_server_conn_id(server_addr, server_port, conn_id);
}

Status rdma_set_header(BurstParams* burst, RDMAOpCode op_code, uintptr_t conn_id, bool is_server,
                       int num_pkts, uint64_t wr_id, const std::string& local_mr_name) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->rdma_set_header(
      burst, op_code, conn_id, is_server, num_pkts, wr_id, local_mr_name);
}

RDMAOpCode rdma_get_opcode(BurstParams* burst) {
  ASSERT_ANO_MGR_INITIALIZED();
  return g_ano_mgr->rdma_get_opcode(burst);
}

};  // namespace holoscan::advanced_network
