/*****************************************************************************
 *
 * Copyright (c) 2012 - 2018 Samsung Electronics Co., Ltd. All rights reserved
 *
 ****************************************************************************/
#ifndef __SLSI_NL80211_VENDOR_H_
#define __SLSI_NL80211_VENDOR_H_

#define OUI_GOOGLE                                      0x001A11
#define OUI_SAMSUNG                                     0x0000f0
#define SLSI_NL80211_GSCAN_SUBCMD_RANGE_START           0x1000
#define SLSI_NL80211_GSCAN_EVENT_RANGE_START            0x01
#define SLSI_NL80211_LOGGING_SUBCMD_RANGE_START           0x1400
#define SLSI_NL80211_NAN_SUBCMD_RANGE_START             0x1500
#define SLSI_NL80211_SOFTAP_SUBCMD_RANGE_START             0x1600
#define SLSI_GSCAN_SCAN_ID_START                        0x410
#define SLSI_GSCAN_SCAN_ID_END                          0x500

#define SLSI_GSCAN_MAX_BUCKETS                          (8)
#define SLSI_GSCAN_MAX_CHANNELS                         (16) /* As per gscan.h */
#define SLSI_GSCAN_MAX_HOTLIST_APS                      (64)
#define SLSI_GSCAN_MAX_BUCKETS_PER_GSCAN                (SLSI_GSCAN_MAX_BUCKETS)
#define SLSI_GSCAN_MAX_SCAN_CACHE_SIZE                  (12000)
#define SLSI_GSCAN_MAX_AP_CACHE_PER_SCAN                (16)
#define SLSI_GSCAN_MAX_SCAN_REPORTING_THRESHOLD         (100)
#define SLSI_GSCAN_MAX_SIGNIFICANT_CHANGE_APS           (64)
#define SLSI_GSCAN_MAX_EPNO_SSIDS                       (32)
#define SLSI_GSCAN_MAX_EPNO_HS2_PARAM                   (8) /* Framework is not using this. Tune when needed */

#define SLSI_REPORT_EVENTS_NONE                         (0)
#define SLSI_REPORT_EVENTS_EACH_SCAN                    (1)
#define SLSI_REPORT_EVENTS_FULL_RESULTS                 (2)
#define SLSI_REPORT_EVENTS_NO_BATCH                     (4)

#define SLSI_NL_ATTRIBUTE_U32_LEN                       (NLA_HDRLEN + 4)
#define SLSI_NL_ATTRIBUTE_COUNTRY_CODE                  (4)
#define SLSI_NL_VENDOR_ID_OVERHEAD                      SLSI_NL_ATTRIBUTE_U32_LEN
#define SLSI_NL_VENDOR_SUBCMD_OVERHEAD                  SLSI_NL_ATTRIBUTE_U32_LEN
#define SLSI_NL_VENDOR_DATA_OVERHEAD                    (NLA_HDRLEN)

#define SLSI_NL_VENDOR_REPLY_OVERHEAD                   (SLSI_NL_VENDOR_ID_OVERHEAD + \
							 SLSI_NL_VENDOR_SUBCMD_OVERHEAD + \
							 SLSI_NL_VENDOR_DATA_OVERHEAD)

#define SLSI_GSCAN_RTT_UNSPECIFIED                      (-1)
#define SLSI_GSCAN_HASH_TABLE_SIZE                      (32)
#define SLSI_GSCAN_HASH_KEY_MASK                        (0x1F)
#define SLSI_GSCAN_GET_HASH_KEY(_key)                   (_key & SLSI_GSCAN_HASH_KEY_MASK)

#define SLSI_KEEP_SCAN_RESULT                           (0)
#define SLSI_DISCARD_SCAN_RESULT                        (1)

#define SLSI_GSCAN_MAX_BSSID_PER_IE                     (20)

#define SLSI_LLS_CAPABILITY_QOS          0x00000001     /* set for QOS association */
#define SLSI_LLS_CAPABILITY_PROTECTED    0x00000002     /* set for protected association (802.11 beacon frame control protected bit set)*/
#define SLSI_LLS_CAPABILITY_INTERWORKING 0x00000004     /* set if 802.11 Extended Capabilities element interworking bit is set*/
#define SLSI_LLS_CAPABILITY_HS20         0x00000008     /* set for HS20 association*/
#define SLSI_LLS_CAPABILITY_SSID_UTF8    0x00000010     /* set is 802.11 Extended Capabilities element UTF-8 SSID bit is set*/
#define SLSI_LLS_CAPABILITY_COUNTRY      0x00000020     /* set is 802.11 Country Element is present*/

#define TIMESPEC_TO_US(ts)  (((u64)(ts).tv_sec * USEC_PER_SEC) + (ts).tv_nsec / NSEC_PER_USEC)

/* Feature enums */
#define SLSI_WIFI_HAL_FEATURE_INFRA              0x000001      /* Basic infrastructure mode */
#define SLSI_WIFI_HAL_FEATURE_INFRA_5G           0x000002      /* Support for 5 GHz Band */
#define SLSI_WIFI_HAL_FEATURE_HOTSPOT            0x000004      /* Support for GAS/ANQP */
#define SLSI_WIFI_HAL_FEATURE_P2P                0x000008      /* Wifi-Direct */
#define SLSI_WIFI_HAL_FEATURE_SOFT_AP            0x000010      /* Soft AP */
#define SLSI_WIFI_HAL_FEATURE_GSCAN              0x000020      /* Google-Scan APIs */
#define SLSI_WIFI_HAL_FEATURE_NAN                0x000040      /* Neighbor Awareness Networking */
#define SLSI_WIFI_HAL_FEATURE_D2D_RTT            0x000080      /* Device-to-device RTT */
#define SLSI_WIFI_HAL_FEATURE_D2AP_RTT           0x000100      /* Device-to-AP RTT */
#define SLSI_WIFI_HAL_FEATURE_BATCH_SCAN         0x000200      /* Batched Scan (legacy) */
#define SLSI_WIFI_HAL_FEATURE_PNO                0x000400      /* Preferred network offload */
#define SLSI_WIFI_HAL_FEATURE_ADDITIONAL_STA     0x000800      /* Support for two STAs */
#define SLSI_WIFI_HAL_FEATURE_TDLS               0x001000      /* Tunnel directed link setup */
#define SLSI_WIFI_HAL_FEATURE_TDLS_OFFCHANNEL    0x002000      /* Support for TDLS off channel */
#define SLSI_WIFI_HAL_FEATURE_EPR                0x004000      /* Enhanced power reporting */
#define SLSI_WIFI_HAL_FEATURE_AP_STA             0x008000      /* Support for AP STA Concurrency */
#define SLSI_WIFI_HAL_FEATURE_LINK_LAYER_STATS   0x010000      /* Link layer stats collection */
#define SLSI_WIFI_HAL_FEATURE_LOGGER             0x020000      /* WiFi Logger */
#define SLSI_WIFI_HAL_FEATURE_HAL_EPNO           0x040000      /* WiFi PNO enhanced */
#define SLSI_WIFI_HAL_FEATURE_RSSI_MONITOR       0x080000      /* RSSI Monitor */
#define SLSI_WIFI_HAL_FEATURE_MKEEP_ALIVE        0x100000      /* WiFi mkeep_alive */
#define SLSI_WIFI_HAL_FEATURE_CONTROL_ROAMING    0x800000      /* Enable/Disable firmware roaming macro */

enum slsi_wifi_attr {
	SLSI_NL_ATTRIBUTE_ND_OFFLOAD_VALUE = 0,
	SLSI_NL_ATTRIBUTE_PNO_RANDOM_MAC_OUI
};

enum SLSI_ROAM_ATTRIBUTES {
	SLSI_NL_ATTR_MAX_BLACKLIST_SIZE,
	SLSI_NL_ATTR_MAX_WHITELIST_SIZE,
	SLSI_NL_ATTR_ROAM_STATE
};

enum slsi_wifi_softap_attr {
	SLSI_NL_ATTRIBUTE_AP_IFACE_NAME
};

enum SLSI_NAN_REPLY_ATTRIBUTES {
	NAN_REPLY_ATTR_STATUS_TYPE,
	NAN_REPLY_ATTR_VALUE,
	NAN_REPLY_ATTR_RESPONSE_TYPE,
	NAN_REPLY_ATTR_PUBLISH_SUBSCRIBE_TYPE,
	NAN_REPLY_ATTR_CAP_MAX_CONCURRENT_CLUSTER,
	NAN_REPLY_ATTR_CAP_MAX_PUBLISHES,
	NAN_REPLY_ATTR_CAP_MAX_SUBSCRIBES,
	NAN_REPLY_ATTR_CAP_MAX_SERVICE_NAME_LEN,
	NAN_REPLY_ATTR_CAP_MAX_MATCH_FILTER_LEN,
	NAN_REPLY_ATTR_CAP_MAX_TOTAL_MATCH_FILTER_LEN,
	NAN_REPLY_ATTR_CAP_MAX_SERVICE_SPECIFIC_INFO_LEN,
	NAN_REPLY_ATTR_CAP_MAX_VSA_DATA_LEN,
	NAN_REPLY_ATTR_CAP_MAX_MESH_DATA_LEN,
	NAN_REPLY_ATTR_CAP_MAX_NDI_INTERFACES,
	NAN_REPLY_ATTR_CAP_MAX_NDP_SESSIONS,
	NAN_REPLY_ATTR_CAP_MAX_APP_INFO_LEN
};

enum SLSI_NAN_REQ_ATTRIBUTES {
	NAN_REQ_ATTR_MASTER_PREF,
	NAN_REQ_ATTR_CLUSTER_LOW,
	NAN_REQ_ATTR_CLUSTER_HIGH,
	NAN_REQ_ATTR_HOP_COUNT_LIMIT_VAL,
	NAN_REQ_ATTR_SID_BEACON_VAL,
	NAN_REQ_ATTR_SUPPORT_2G4_VAL,
	NAN_REQ_ATTR_SUPPORT_5G_VAL,
	NAN_REQ_ATTR_RSSI_CLOSE_2G4_VAL,
	NAN_REQ_ATTR_RSSI_MIDDLE_2G4_VAL,
	NAN_REQ_ATTR_RSSI_PROXIMITY_2G4_VAL,
	NAN_REQ_ATTR_BEACONS_2G4_VAL,
	NAN_REQ_ATTR_SDF_2G4_VAL,
	NAN_REQ_ATTR_CHANNEL_2G4_MHZ_VAL,
	NAN_REQ_ATTR_RSSI_PROXIMITY_VAL,
	NAN_REQ_ATTR_RSSI_CLOSE_5G_VAL,
	NAN_REQ_ATTR_RSSI_CLOSE_PROXIMITY_5G_VAL,
	NAN_REQ_ATTR_RSSI_MIDDLE_5G_VAL,
	NAN_REQ_ATTR_RSSI_PROXIMITY_5G_VAL,
	NAN_REQ_ATTR_BEACON_5G_VAL,
	NAN_REQ_ATTR_SDF_5G_VAL,
	NAN_REQ_ATTR_CHANNEL_5G_MHZ_VAL,
	NAN_REQ_ATTR_RSSI_WINDOW_SIZE_VAL,
	NAN_REQ_ATTR_OUI_VAL,
	NAN_REQ_ATTR_MAC_ADDR_VAL,
	NAN_REQ_ATTR_CLUSTER_VAL,
	NAN_REQ_ATTR_SOCIAL_CH_SCAN_DWELL_TIME,
	NAN_REQ_ATTR_SOCIAL_CH_SCAN_PERIOD,
	NAN_REQ_ATTR_RANDOM_FACTOR_FORCE_VAL,
	NAN_REQ_ATTR_HOP_COUNT_FORCE_VAL,
	NAN_REQ_ATTR_CONN_CAPABILITY_PAYLOAD_TX,
	NAN_REQ_ATTR_CONN_CAPABILITY_IBSS,
	NAN_REQ_ATTR_CONN_CAPABILITY_WFD,
	NAN_REQ_ATTR_CONN_CAPABILITY_WFDS,
	NAN_REQ_ATTR_CONN_CAPABILITY_TDLS,
	NAN_REQ_ATTR_CONN_CAPABILITY_MESH,
	NAN_REQ_ATTR_CONN_CAPABILITY_WLAN_INFRA,
	NAN_REQ_ATTR_DISCOVERY_ATTR_NUM_ENTRIES,
	NAN_REQ_ATTR_DISCOVERY_ATTR_VAL,
	NAN_REQ_ATTR_CONN_TYPE,
	NAN_REQ_ATTR_NAN_ROLE,
	NAN_REQ_ATTR_TRANSMIT_FREQ,
	NAN_REQ_ATTR_AVAILABILITY_DURATION,
	NAN_REQ_ATTR_AVAILABILITY_INTERVAL,
	NAN_REQ_ATTR_MESH_ID_LEN,
	NAN_REQ_ATTR_MESH_ID,
	NAN_REQ_ATTR_INFRASTRUCTURE_SSID_LEN,
	NAN_REQ_ATTR_INFRASTRUCTURE_SSID,
	NAN_REQ_ATTR_FURTHER_AVAIL_NUM_ENTRIES,
	NAN_REQ_ATTR_FURTHER_AVAIL_VAL,
	NAN_REQ_ATTR_FURTHER_AVAIL_ENTRY_CTRL,
	NAN_REQ_ATTR_FURTHER_AVAIL_CHAN_CLASS,
	NAN_REQ_ATTR_FURTHER_AVAIL_CHAN,
	NAN_REQ_ATTR_FURTHER_AVAIL_CHAN_MAPID,
	NAN_REQ_ATTR_FURTHER_AVAIL_INTERVAL_BITMAP,
	NAN_REQ_ATTR_PUBLISH_ID,
	NAN_REQ_ATTR_PUBLISH_TTL,
	NAN_REQ_ATTR_PUBLISH_PERIOD,
	NAN_REQ_ATTR_PUBLISH_TYPE,
	NAN_REQ_ATTR_PUBLISH_TX_TYPE,
	NAN_REQ_ATTR_PUBLISH_COUNT,
	NAN_REQ_ATTR_PUBLISH_SERVICE_NAME_LEN,
	NAN_REQ_ATTR_PUBLISH_SERVICE_NAME,
	NAN_REQ_ATTR_PUBLISH_MATCH_ALGO,
	NAN_REQ_ATTR_PUBLISH_SERVICE_INFO_LEN,
	NAN_REQ_ATTR_PUBLISH_SERVICE_INFO,
	NAN_REQ_ATTR_PUBLISH_RX_MATCH_FILTER_LEN,
	NAN_REQ_ATTR_PUBLISH_RX_MATCH_FILTER,
	NAN_REQ_ATTR_PUBLISH_TX_MATCH_FILTER_LEN,
	NAN_REQ_ATTR_PUBLISH_TX_MATCH_FILTER,
	NAN_REQ_ATTR_PUBLISH_RSSI_THRESHOLD_FLAG,
	NAN_REQ_ATTR_PUBLISH_CONN_MAP,
	NAN_REQ_ATTR_PUBLISH_RECV_IND_CFG,
	NAN_REQ_ATTR_SUBSCRIBE_ID,
	NAN_REQ_ATTR_SUBSCRIBE_TTL,
	NAN_REQ_ATTR_SUBSCRIBE_PERIOD,
	NAN_REQ_ATTR_SUBSCRIBE_TYPE,
	NAN_REQ_ATTR_SUBSCRIBE_RESP_FILTER_TYPE,
	NAN_REQ_ATTR_SUBSCRIBE_RESP_INCLUDE,
	NAN_REQ_ATTR_SUBSCRIBE_USE_RESP_FILTER,
	NAN_REQ_ATTR_SUBSCRIBE_SSI_REQUIRED,
	NAN_REQ_ATTR_SUBSCRIBE_MATCH_INDICATOR,
	NAN_REQ_ATTR_SUBSCRIBE_COUNT,
	NAN_REQ_ATTR_SUBSCRIBE_SERVICE_NAME_LEN,
	NAN_REQ_ATTR_SUBSCRIBE_SERVICE_NAME,
	NAN_REQ_ATTR_SUBSCRIBE_SERVICE_INFO_LEN,
	NAN_REQ_ATTR_SUBSCRIBE_SERVICE_INFO,
	NAN_REQ_ATTR_SUBSCRIBE_RX_MATCH_FILTER_LEN,
	NAN_REQ_ATTR_SUBSCRIBE_RX_MATCH_FILTER,
	NAN_REQ_ATTR_SUBSCRIBE_TX_MATCH_FILTER_LEN,
	NAN_REQ_ATTR_SUBSCRIBE_TX_MATCH_FILTER,
	NAN_REQ_ATTR_SUBSCRIBE_RSSI_THRESHOLD_FLAG,
	NAN_REQ_ATTR_SUBSCRIBE_CONN_MAP,
	NAN_REQ_ATTR_SUBSCRIBE_NUM_INTF_ADDR_PRESENT,
	NAN_REQ_ATTR_SUBSCRIBE_INTF_ADDR,
	NAN_REQ_ATTR_SUBSCRIBE_RECV_IND_CFG,
	NAN_REQ_ATTR_FOLLOWUP_ID,
	NAN_REQ_ATTR_FOLLOWUP_REQUESTOR_ID,
	NAN_REQ_ATTR_FOLLOWUP_ADDR,
	NAN_REQ_ATTR_FOLLOWUP_PRIORITY,
	NAN_REQ_ATTR_FOLLOWUP_SERVICE_NAME_LEN,
	NAN_REQ_ATTR_FOLLOWUP_SERVICE_NAME,
	NAN_REQ_ATTR_FOLLOWUP_TX_WINDOW,
	NAN_REQ_ATTR_FOLLOWUP_RECV_IND_CFG
};

enum SLSI_NAN_RESP_ATTRIBUTES {
	NAN_RESP_ATTR_MAX_CONCURRENT_NAN_CLUSTERS,
	NAN_RESP_ATTR_MAX_PUBLISHES,
	NAN_RESP_ATTR_MAX_SUBSCRIBES,
	NAN_RESP_ATTR_MAX_SERVICE_NAME_LEN,
	NAN_RESP_ATTR_MAX_MATCH_FILTER_LEN,
	NAN_RESP_ATTR_MAX_TOTAL_MATCH_FILTER_LEN,
	NAN_RESP_ATTR_MAX_SERVICE_SPECIFIC_INFO_LEN,
	NAN_RESP_ATTR_MAX_VSA_DATA_LEN,
	NAN_RESP_ATTR_MAX_MESH_DATA_LEN,
	NAN_RESP_ATTR_MAX_NDI_INTERFACES,
	NAN_RESP_ATTR_MAX_NDP_SESSIONS,
	NAN_RESP_ATTR_MAX_APP_INFO_LEN,
	NAN_RESP_ATTR_SUBSCRIBE_ID,
	NAN_RESP_ATTR_PUBLISH_ID
};

enum SLSI_NAN_EVT_ATTRIBUTES {
	NAN_EVT_ATTR_MATCH_PUBLISH_SUBSCRIBE_ID,
	NAN_EVT_ATTR_MATCH_REQUESTOR_INSTANCE_ID,
	NAN_EVT_ATTR_MATCH_ADDR,
	NAN_EVT_ATTR_MATCH_SERVICE_SPECIFIC_INFO_LEN,
	NAN_EVT_ATTR_MATCH_SERVICE_SPECIFIC_INFO,
	NAN_EVT_ATTR_MATCH_SDF_MATCH_FILTER_LEN,
	NAN_EVT_ATTR_MATCH_SDF_MATCH_FILTER,
	NAN_EVT_ATTR_MATCH_MATCH_OCCURRED_FLAG,
	NAN_EVT_ATTR_MATCH_OUT_OF_RESOURCE_FLAG,
	NAN_EVT_ATTR_MATCH_RSSI_VALUE,
	/*CONN_CAPABILITY*/
	NAN_EVT_ATTR_MATCH_CONN_CAPABILITY_IS_WFD_SUPPORTED,
	NAN_EVT_ATTR_MATCH_CONN_CAPABILITY_IS_WFDS_SUPPORTED,
	NAN_EVT_ATTR_MATCH_CONN_CAPABILITY_IS_TDLS_SUPPORTED,
	NAN_EVT_ATTR_MATCH_CONN_CAPABILITY_IS_IBSS_SUPPORTED,
	NAN_EVT_ATTR_MATCH_CONN_CAPABILITY_IS_MESH_SUPPORTED,
	NAN_EVT_ATTR_MATCH_CONN_CAPABILITY_WLAN_INFRA_FIELD,
	NAN_EVT_ATTR_MATCH_NUM_RX_DISCOVERY_ATTR,
	NAN_EVT_ATTR_MATCH_RX_DISCOVERY_ATTR,
	/*NANRECEIVEPOSTDISCOVERY DISCOVERY_ATTR,*/
	NAN_EVT_ATTR_MATCH_DISC_ATTR_TYPE,
	NAN_EVT_ATTR_MATCH_DISC_ATTR_ROLE,
	NAN_EVT_ATTR_MATCH_DISC_ATTR_DURATION,
	NAN_EVT_ATTR_MATCH_DISC_ATTR_AVAIL_INTERVAL_BITMAP,
	NAN_EVT_ATTR_MATCH_DISC_ATTR_MAPID,
	NAN_EVT_ATTR_MATCH_DISC_ATTR_ADDR,
	NAN_EVT_ATTR_MATCH_DISC_ATTR_MESH_ID_LEN,
	NAN_EVT_ATTR_MATCH_DISC_ATTR_MESH_ID,
	NAN_EVT_ATTR_MATCH_DISC_ATTR_INFRASTRUCTURE_SSID_LEN,
	NAN_EVT_ATTR_MATCH_DISC_ATTR_INFRASTRUCTURE_SSID_VAL,
	NAN_EVT_ATTR_MATCH_NUM_CHANS,
	NAN_EVT_ATTR_MATCH_FAMCHAN,
	/*FAMCHAN[32],*/
	NAN_EVT_ATTR_MATCH_FAM_ENTRY_CONTROL,
	NAN_EVT_ATTR_MATCH_FAM_CLASS_VAL,
	NAN_EVT_ATTR_MATCH_FAM_CHANNEL,
	NAN_EVT_ATTR_MATCH_FAM_MAPID,
	NAN_EVT_ATTR_MATCH_FAM_AVAIL_INTERVAL_BITMAP,
	NAN_EVT_ATTR_MATCH_CLUSTER_ATTRIBUTE_LEN,
	NAN_EVT_ATTR_MATCH_CLUSTER_ATTRIBUTE,
	NAN_EVT_ATTR_PUBLISH_ID,
	NAN_EVT_ATTR_PUBLISH_REASON,
	NAN_EVT_ATTR_SUBSCRIBE_ID,
	NAN_EVT_ATTR_SUBSCRIBE_REASON,
	NAN_EVT_ATTR_DISABLED_REASON,
	NAN_EVT_ATTR_FOLLOWUP_PUBLISH_SUBSCRIBE_ID,
	NAN_EVT_ATTR_FOLLOWUP_REQUESTOR_INSTANCE_ID,
	NAN_EVT_ATTR_FOLLOWUP_ADDR,
	NAN_EVT_ATTR_FOLLOWUP_DW_OR_FAW,
	NAN_EVT_ATTR_FOLLOWUP_SERVICE_SPECIFIC_INFO_LEN,
	NAN_EVT_ATTR_FOLLOWUP_SERVICE_SPECIFIC_INFO,
	NAN_EVT_ATTR_DISCOVERY_ENGINE_EVT_TYPE,
	NAN_EVT_ATTR_DISCOVERY_ENGINE_MAC_ADDR,
	NAN_EVT_ATTR_DISCOVERY_ENGINE_CLUSTER
};

enum GSCAN_ATTRIBUTE {
	GSCAN_ATTRIBUTE_NUM_BUCKETS = 10,
	GSCAN_ATTRIBUTE_BASE_PERIOD,
	GSCAN_ATTRIBUTE_BUCKETS_BAND,
	GSCAN_ATTRIBUTE_BUCKET_ID,
	GSCAN_ATTRIBUTE_BUCKET_PERIOD,
	GSCAN_ATTRIBUTE_BUCKET_NUM_CHANNELS,
	GSCAN_ATTRIBUTE_BUCKET_CHANNELS,
	GSCAN_ATTRIBUTE_NUM_AP_PER_SCAN,
	GSCAN_ATTRIBUTE_REPORT_THRESHOLD,
	GSCAN_ATTRIBUTE_NUM_SCANS_TO_CACHE,
	GSCAN_ATTRIBUTE_REPORT_THRESHOLD_NUM_SCANS,
	GSCAN_ATTRIBUTE_BAND = GSCAN_ATTRIBUTE_BUCKETS_BAND,

	GSCAN_ATTRIBUTE_ENABLE_FEATURE = 20,
	GSCAN_ATTRIBUTE_SCAN_RESULTS_COMPLETE, /* indicates no more results */
	GSCAN_ATTRIBUTE_REPORT_EVENTS,

	/* remaining reserved for additional attributes */
	GSCAN_ATTRIBUTE_NUM_OF_RESULTS = 30,
	GSCAN_ATTRIBUTE_SCAN_RESULTS, /* flat array of wifi_scan_result */
	GSCAN_ATTRIBUTE_NUM_CHANNELS,
	GSCAN_ATTRIBUTE_CHANNEL_LIST,
	GSCAN_ATTRIBUTE_SCAN_ID,
	GSCAN_ATTRIBUTE_SCAN_FLAGS,
	GSCAN_ATTRIBUTE_SCAN_BUCKET_BIT,

	/* remaining reserved for additional attributes */
	GSCAN_ATTRIBUTE_SSID = 40,
	GSCAN_ATTRIBUTE_BSSID,
	GSCAN_ATTRIBUTE_CHANNEL,
	GSCAN_ATTRIBUTE_RSSI,
	GSCAN_ATTRIBUTE_TIMESTAMP,
	GSCAN_ATTRIBUTE_RTT,
	GSCAN_ATTRIBUTE_RTTSD,

	/* remaining reserved for additional attributes */
	GSCAN_ATTRIBUTE_HOTLIST_BSSIDS = 50,
	GSCAN_ATTRIBUTE_RSSI_LOW,
	GSCAN_ATTRIBUTE_RSSI_HIGH,
	GSCAN_ATTRIBUTE_HOTLIST_ELEM,
	GSCAN_ATTRIBUTE_HOTLIST_FLUSH,
	GSCAN_ATTRIBUTE_CHANNEL_NUMBER,

	/* remaining reserved for additional attributes */
	GSCAN_ATTRIBUTE_RSSI_SAMPLE_SIZE = 60,
	GSCAN_ATTRIBUTE_LOST_AP_SAMPLE_SIZE,
	GSCAN_ATTRIBUTE_MIN_BREACHING,
	GSCAN_ATTRIBUTE_SIGNIFICANT_CHANGE_BSSIDS,

	GSCAN_ATTRIBUTE_BUCKET_STEP_COUNT = 70,
	GSCAN_ATTRIBUTE_BUCKET_EXPONENT,
	GSCAN_ATTRIBUTE_BUCKET_MAX_PERIOD,

	GSCAN_ATTRIBUTE_NUM_BSSID,
	GSCAN_ATTRIBUTE_BLACKLIST_BSSID,

	GSCAN_ATTRIBUTE_MAX
};

enum epno_ssid_attribute {
	SLSI_ATTRIBUTE_EPNO_MINIMUM_5G_RSSI,
	SLSI_ATTRIBUTE_EPNO_MINIMUM_2G_RSSI,
	SLSI_ATTRIBUTE_EPNO_INITIAL_SCORE_MAX,
	SLSI_ATTRIBUTE_EPNO_CUR_CONN_BONUS,
	SLSI_ATTRIBUTE_EPNO_SAME_NETWORK_BONUS,
	SLSI_ATTRIBUTE_EPNO_SECURE_BONUS,
	SLSI_ATTRIBUTE_EPNO_5G_BONUS,
	SLSI_ATTRIBUTE_EPNO_SSID_NUM,
	SLSI_ATTRIBUTE_EPNO_SSID_LIST,
	SLSI_ATTRIBUTE_EPNO_SSID,
	SLSI_ATTRIBUTE_EPNO_SSID_LEN,
	SLSI_ATTRIBUTE_EPNO_FLAGS,
	SLSI_ATTRIBUTE_EPNO_AUTH,
	SLSI_ATTRIBUTE_EPNO_MAX
};

enum epno_hs_attribute {
	SLSI_ATTRIBUTE_EPNO_HS_PARAM_LIST,
	SLSI_ATTRIBUTE_EPNO_HS_NUM,
	SLSI_ATTRIBUTE_EPNO_HS_ID,
	SLSI_ATTRIBUTE_EPNO_HS_REALM,
	SLSI_ATTRIBUTE_EPNO_HS_CONSORTIUM_IDS,
	SLSI_ATTRIBUTE_EPNO_HS_PLMN,
	SLSI_ATTRIBUTE_EPNO_HS_MAX
};

enum gscan_bucket_attributes {
	GSCAN_ATTRIBUTE_CH_BUCKET_1,
	GSCAN_ATTRIBUTE_CH_BUCKET_2,
	GSCAN_ATTRIBUTE_CH_BUCKET_3,
	GSCAN_ATTRIBUTE_CH_BUCKET_4,
	GSCAN_ATTRIBUTE_CH_BUCKET_5,
	GSCAN_ATTRIBUTE_CH_BUCKET_6,
	GSCAN_ATTRIBUTE_CH_BUCKET_7,
	GSCAN_ATTRIBUTE_CH_BUCKET_8
};

enum wifi_band {
	WIFI_BAND_UNSPECIFIED,
	WIFI_BAND_BG = 1,                       /* 2.4 GHz */
	WIFI_BAND_A = 2,                        /* 5 GHz without DFS */
	WIFI_BAND_A_DFS = 4,                    /* 5 GHz DFS only */
	WIFI_BAND_A_WITH_DFS = 6,               /* 5 GHz with DFS */
	WIFI_BAND_ABG = 3,                      /* 2.4 GHz + 5 GHz; no DFS */
	WIFI_BAND_ABG_WITH_DFS = 7,             /* 2.4 GHz + 5 GHz with DFS */
};

enum wifi_scan_event {
	WIFI_SCAN_RESULTS_AVAILABLE,
	WIFI_SCAN_THRESHOLD_NUM_SCANS,
	WIFI_SCAN_THRESHOLD_PERCENT,
	WIFI_SCAN_FAILED,
};

enum wifi_mkeep_alive_attribute {
	MKEEP_ALIVE_ATTRIBUTE_ID,
	MKEEP_ALIVE_ATTRIBUTE_IP_PKT,
	MKEEP_ALIVE_ATTRIBUTE_IP_PKT_LEN,
	MKEEP_ALIVE_ATTRIBUTE_SRC_MAC_ADDR,
	MKEEP_ALIVE_ATTRIBUTE_DST_MAC_ADDR,
	MKEEP_ALIVE_ATTRIBUTE_PERIOD_MSEC
};

enum wifi_rssi_monitor_attr {
	SLSI_RSSI_MONITOR_ATTRIBUTE_MAX_RSSI,
	SLSI_RSSI_MONITOR_ATTRIBUTE_MIN_RSSI,
	SLSI_RSSI_MONITOR_ATTRIBUTE_START
};

enum lls_attribute {
	LLS_ATTRIBUTE_SET_MPDU_SIZE_THRESHOLD = 1,
	LLS_ATTRIBUTE_SET_AGGR_STATISTICS_GATHERING,
	LLS_ATTRIBUTE_CLEAR_STOP_REQUEST_MASK,
	LLS_ATTRIBUTE_CLEAR_STOP_REQUEST,
	LLS_ATTRIBUTE_MAX
};

enum slsi_hal_vendor_subcmds {
	SLSI_NL80211_VENDOR_SUBCMD_GET_CAPABILITIES = SLSI_NL80211_GSCAN_SUBCMD_RANGE_START,
	SLSI_NL80211_VENDOR_SUBCMD_GET_VALID_CHANNELS,
	SLSI_NL80211_VENDOR_SUBCMD_ADD_GSCAN,
	SLSI_NL80211_VENDOR_SUBCMD_DEL_GSCAN,
	SLSI_NL80211_VENDOR_SUBCMD_GET_SCAN_RESULTS,
	SLSI_NL80211_VENDOR_SUBCMD_SET_BSSID_HOTLIST,
	SLSI_NL80211_VENDOR_SUBCMD_RESET_BSSID_HOTLIST,
	SLSI_NL80211_VENDOR_SUBCMD_GET_HOTLIST_RESULTS,
	SLSI_NL80211_VENDOR_SUBCMD_SET_SIGNIFICANT_CHANGE,
	SLSI_NL80211_VENDOR_SUBCMD_RESET_SIGNIFICANT_CHANGE,
	SLSI_NL80211_VENDOR_SUBCMD_SET_GSCAN_OUI,
	SLSI_NL80211_VENDOR_SUBCMD_SET_NODFS,
	SLSI_NL80211_VENDOR_SUBCMD_START_KEEP_ALIVE_OFFLOAD,
	SLSI_NL80211_VENDOR_SUBCMD_STOP_KEEP_ALIVE_OFFLOAD,
	SLSI_NL80211_VENDOR_SUBCMD_SET_BSSID_BLACKLIST,
	SLSI_NL80211_VENDOR_SUBCMD_SET_EPNO_LIST,
	SLSI_NL80211_VENDOR_SUBCMD_SET_HS_LIST,
	SLSI_NL80211_VENDOR_SUBCMD_RESET_HS_LIST,
	SLSI_NL80211_VENDOR_SUBCMD_SET_RSSI_MONITOR,
	SLSI_NL80211_VENDOR_SUBCMD_LSTATS_SUBCMD_SET_STATS,
	SLSI_NL80211_VENDOR_SUBCMD_LSTATS_SUBCMD_GET_STATS,
	SLSI_NL80211_VENDOR_SUBCMD_LSTATS_SUBCMD_CLEAR_STATS,
	SLSI_NL80211_VENDOR_SUBCMD_GET_FEATURE_SET,
	SLSI_NL80211_VENDOR_SUBCMD_SET_COUNTRY_CODE,
	SLSI_NL80211_VENDOR_SUBCMD_CONFIGURE_ND_OFFLOAD,
	SLSI_NL80211_VENDOR_SUBCMD_GET_ROAMING_CAPABILITIES,
	SLSI_NL80211_VENDOR_SUBCMD_SET_ROAMING_STATE,
	SLSI_NL80211_VENDOR_SUBCMD_START_LOGGING = SLSI_NL80211_LOGGING_SUBCMD_RANGE_START,
	SLSI_NL80211_VENDOR_SUBCMD_TRIGGER_FW_MEM_DUMP,
	SLSI_NL80211_VENDOR_SUBCMD_GET_FW_MEM_DUMP,
	SLSI_NL80211_VENDOR_SUBCMD_GET_VERSION,
	SLSI_NL80211_VENDOR_SUBCMD_GET_RING_STATUS,
	SLSI_NL80211_VENDOR_SUBCMD_GET_RING_DATA,
	SLSI_NL80211_VENDOR_SUBCMD_GET_FEATURE,
	SLSI_NL80211_VENDOR_SUBCMD_RESET_LOGGING,
	SLSI_NL80211_VENDOR_SUBCMD_TRIGGER_DRIVER_MEM_DUMP,
	SLSI_NL80211_VENDOR_SUBCMD_GET_DRIVER_MEM_DUMP,
	SLSI_NL80211_VENDOR_SUBCMD_START_PKT_FATE_MONITORING,
	SLSI_NL80211_VENDOR_SUBCMD_GET_TX_PKT_FATES,
	SLSI_NL80211_VENDOR_SUBCMD_GET_RX_PKT_FATES,
	SLSI_NL80211_VENDOR_SUBCMD_GET_WAKE_REASON_STATS,
	SLSI_NL80211_VENDOR_SUBCMD_NAN_ENABLE = SLSI_NL80211_NAN_SUBCMD_RANGE_START,
	SLSI_NL80211_VENDOR_SUBCMD_NAN_DISABLE,
	SLSI_NL80211_VENDOR_SUBCMD_NAN_PUBLISH,
	SLSI_NL80211_VENDOR_SUBCMD_NAN_PUBLISHCANCEL,
	SLSI_NL80211_VENDOR_SUBCMD_NAN_SUBSCRIBE,
	SLSI_NL80211_VENDOR_SUBCMD_NAN_SUBSCRIBECANCEL,
	SLSI_NL80211_VENDOR_SUBCMD_NAN_TXFOLLOWUP,
	SLSI_NL80211_VENDOR_SUBCMD_NAN_CONFIG,
	SLSI_NL80211_VENDOR_SUBCMD_NAN_CAPABILITIES,
	SLSI_NL80211_VENDOR_SUBCMD_CREATE_SOFTAP_INTERFACE = SLSI_NL80211_SOFTAP_SUBCMD_RANGE_START,
	SLSI_NL80211_VENDOR_SUBCMD_DELETE_SOFTAP_INTERFACE
};

enum slsi_supp_vendor_subcmds {
	SLSI_NL80211_VENDOR_SUBCMD_UNSPEC = 0,
	SLSI_NL80211_VENDOR_SUBCMD_KEY_MGMT_SET_KEY,
};

enum slsi_vendor_event_values {
	SLSI_NL80211_SIGNIFICANT_CHANGE_EVENT,
	SLSI_NL80211_HOTLIST_AP_FOUND_EVENT,
	SLSI_NL80211_SCAN_RESULTS_AVAILABLE_EVENT,
	SLSI_NL80211_FULL_SCAN_RESULT_EVENT,
	SLSI_NL80211_SCAN_EVENT,
	SLSI_NL80211_HOTLIST_AP_LOST_EVENT,
	SLSI_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH,
	SLSI_NL80211_VENDOR_HANGED_EVENT,
	SLSI_NL80211_EPNO_EVENT,
	SLSI_NL80211_HOTSPOT_MATCH,
	SLSI_NL80211_RSSI_REPORT_EVENT,
	SLSI_NL80211_LOGGER_RING_EVENT,
	SLSI_NL80211_LOGGER_FW_DUMP_EVENT,
	SLSI_NL80211_NAN_RESPONSE_EVENT,
	SLSI_NL80211_NAN_PUBLISH_TERMINATED_EVENT,
	SLSI_NL80211_NAN_MATCH_EVENT,
	SLSI_NL80211_NAN_MATCH_EXPIRED_EVENT,
	SLSI_NL80211_NAN_SUBSCRIBE_TERMINATED_EVENT,
	SLSI_NL80211_NAN_FOLLOWUP_EVENT,
	SLSI_NL80211_NAN_DISCOVERY_ENGINE_EVENT
};

enum slsi_lls_interface_mode {
	SLSI_LLS_INTERFACE_STA = 0,
	SLSI_LLS_INTERFACE_SOFTAP = 1,
	SLSI_LLS_INTERFACE_IBSS = 2,
	SLSI_LLS_INTERFACE_P2P_CLIENT = 3,
	SLSI_LLS_INTERFACE_P2P_GO = 4,
	SLSI_LLS_INTERFACE_NAN = 5,
	SLSI_LLS_INTERFACE_MESH = 6,
	SLSI_LLS_INTERFACE_UNKNOWN = -1
};

enum slsi_lls_connection_state {
	SLSI_LLS_DISCONNECTED = 0,
	SLSI_LLS_AUTHENTICATING = 1,
	SLSI_LLS_ASSOCIATING = 2,
	SLSI_LLS_ASSOCIATED = 3,
	SLSI_LLS_EAPOL_STARTED = 4,   /* if done by firmware/driver*/
	SLSI_LLS_EAPOL_COMPLETED = 5, /* if done by firmware/driver*/
};

enum slsi_lls_roam_state {
	SLSI_LLS_ROAMING_IDLE = 0,
	SLSI_LLS_ROAMING_ACTIVE = 1,
};

/* access categories */
enum slsi_lls_traffic_ac {
	SLSI_LLS_AC_VO  = 0,
	SLSI_LLS_AC_VI  = 1,
	SLSI_LLS_AC_BE  = 2,
	SLSI_LLS_AC_BK  = 3,
	SLSI_LLS_AC_MAX = 4,
};

/* channel operating width */
enum slsi_lls_channel_width {
	SLSI_LLS_CHAN_WIDTH_20    = 0,
	SLSI_LLS_CHAN_WIDTH_40    = 1,
	SLSI_LLS_CHAN_WIDTH_80    = 2,
	SLSI_LLS_CHAN_WIDTH_160   = 3,
	SLSI_LLS_CHAN_WIDTH_80P80 = 4,
	SLSI_LLS_CHAN_WIDTH_5     = 5,
	SLSI_LLS_CHAN_WIDTH_10    = 6,
	SLSI_LLS_CHAN_WIDTH_INVALID = -1
};

/* wifi peer type */
enum slsi_lls_peer_type {
	SLSI_LLS_PEER_STA,
	SLSI_LLS_PEER_AP,
	SLSI_LLS_PEER_P2P_GO,
	SLSI_LLS_PEER_P2P_CLIENT,
	SLSI_LLS_PEER_NAN,
	SLSI_LLS_PEER_TDLS,
	SLSI_LLS_PEER_INVALID,
};

/* slsi_enhanced_logging_attributes */
enum slsi_enhanced_logging_attributes {
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_DRIVER_VERSION,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_FW_VERSION,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_RING_ID,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_RING_NAME,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_RING_FLAGS,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_VERBOSE_LEVEL,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_LOG_MAX_INTERVAL,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_LOG_MIN_DATA_SIZE,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_FW_DUMP_LEN,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_FW_DUMP_DATA,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_RING_DATA,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_RING_STATUS,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_RING_NUM,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_DRIVER_DUMP_LEN,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_DRIVER_DUMP_DATA,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_PKT_FATE_NUM,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_PKT_FATE_DATA,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_INVALID = 0,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_TOTAL_CMD_EVENT_WAKE,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_CMD_EVENT_WAKE_CNT_PTR,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_CMD_EVENT_WAKE_CNT_SZ,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_TOTAL_DRIVER_FW_LOCAL_WAKE,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_DRIVER_FW_LOCAL_WAKE_CNT_PTR,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_DRIVER_FW_LOCAL_WAKE_CNT_SZ,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_TOTAL_RX_DATA_WAKE,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_RX_UNICAST_CNT,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_RX_MULTICAST_CNT,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_RX_BROADCAST_CNT,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_ICMP_PKT,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_ICMP6_PKT,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_ICMP6_RA,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_ICMP6_NA,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_ICMP6_NS,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_ICMP4_RX_MULTICAST_CNT,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_ICMP6_RX_MULTICAST_CNT,
	SLSI_ENHANCED_LOGGING_ATTRIBUTE_WAKE_STATS_OTHER_RX_MULTICAST_CNT,
};

struct slsi_nl_gscan_capabilities {
	int max_scan_cache_size;
	int max_scan_buckets;
	int max_ap_cache_per_scan;
	int max_rssi_sample_size;
	int max_scan_reporting_threshold;
	int max_hotlist_aps;
	int max_hotlist_ssids;
	int max_significant_wifi_change_aps;
	int max_bssid_history_entries;
	int max_number_epno_networks;
	int max_number_epno_networks_by_ssid;
	int max_number_of_white_listed_ssid;
};

struct slsi_nl_channel_param {
	int channel;
	int dwell_time_ms;
	int passive;         /* 0 => active, 1 => passive scan; ignored for DFS */
};

struct slsi_nl_bucket_param {
	int                          bucket_index;
	enum wifi_band               band;
	int                          period; /* desired period in millisecond */
	u8                           report_events;
	int                          max_period; /* If non-zero: scan period will grow exponentially to a maximum period of max_period */
	int                          exponent;    /* multiplier: new_period = old_period ^ exponent */
	int                          step_count; /* number of scans performed at a given period and until the exponent is applied */
	int                          num_channels;
	struct slsi_nl_channel_param channels[SLSI_GSCAN_MAX_CHANNELS];
};

struct slsi_nl_gscan_param {
	int                         base_period;     /* base timer period in ms */
	int                         max_ap_per_scan; /* number of APs to store in each scan in the BSSID/RSSI history buffer */
	int                         report_threshold_percent; /* when scan_buffer  is this much full, wake up application processor */
	int                         report_threshold_num_scans; /* wake up application processor after these many scans */
	int                         num_buckets;
	struct slsi_nl_bucket_param nl_bucket[SLSI_GSCAN_MAX_BUCKETS];
};

struct slsi_nl_scan_result_param {
	u64 ts;                               /* time since boot (in microsecond) when the result was retrieved */
	u8  ssid[IEEE80211_MAX_SSID_LEN + 1]; /* NULL terminated */
	u8  bssid[6];
	int channel;                          /* channel frequency in MHz */
	int rssi;                             /* in db */
	s64 rtt;                              /* in nanoseconds */
	s64 rtt_sd;                           /* standard deviation in rtt */
	u16 beacon_period;                    /* period advertised in the beacon */
	u16 capability;                       /* capabilities advertised in the beacon */
	u32 ie_length;                        /* size of the ie_data blob */
	u8  ie_data[1];                       /* beacon IE */
};

struct slsi_nl_ap_threshold_param {
	u8  bssid[6];          /* AP BSSID */
	s16 low;               /* low threshold */
	s16 high;              /* high threshold */
};

struct slsi_nl_hotlist_param {
	u8                                lost_ap_sample_size;
	u8                                num_bssid;                          /* number of hotlist APs */
	struct slsi_nl_ap_threshold_param ap[SLSI_GSCAN_MAX_HOTLIST_APS];  /* hotlist APs */
};

struct slsi_bucket {
	bool              used;                /* to identify if this entry is free */
	bool              for_change_tracking; /* Indicates if this scan_id is used for change_tracking */
	u8                report_events;       /* this is received from HAL/Framework */
	u16               scan_id;             /* SLSI_GSCAN_SCAN_ID_START + <offset in the array> */
	int               scan_cycle;          /* To find the current scan cycle */
	struct slsi_gscan *gscan;              /* gscan ref in which this bucket belongs */
};

struct slsi_gscan {
	int                         max_ap_per_scan;  /* received from HAL/Framework */
	int                         report_threshold_percent; /* received from HAL/Framework */
	int                         report_threshold_num_scans; /* received from HAL/Framework */
	int                         num_scans;
	int                         num_buckets;      /* received from HAL/Framework */
	struct slsi_nl_bucket_param nl_bucket;        /* store the first bucket params. used in tracking*/
	struct slsi_bucket          *bucket[SLSI_GSCAN_MAX_BUCKETS_PER_GSCAN];
	struct slsi_gscan           *next;
};

struct slsi_gscan_param {
	struct slsi_nl_bucket_param *nl_bucket;
	struct slsi_bucket          *bucket;
};

struct slsi_nl_significant_change_params {
	int                               rssi_sample_size;    /* number of samples for averaging RSSI */
	int                               lost_ap_sample_size; /* number of samples to confirm AP loss */
	int                               min_breaching;       /* number of APs breaching threshold */
	int                               num_bssid;              /* max 64 */
	struct slsi_nl_ap_threshold_param ap[SLSI_GSCAN_MAX_SIGNIFICANT_CHANGE_APS];
};

struct slsi_gscan_result {
	struct slsi_gscan_result         *hnext;
	int                              scan_cycle;
	int                              scan_res_len;
	int                              anqp_length;
	struct slsi_nl_scan_result_param nl_scan_res;
};

struct slsi_hotlist_result {
	struct list_head                 list;
	int                              scan_res_len;
	struct slsi_nl_scan_result_param nl_scan_res;
};

struct slsi_epno_ssid_param {
	u16 flags;
	u8  ssid_len;
	u8  ssid[32];
};

struct slsi_epno_param {
	u16 min_5g_rssi;                           /* minimum 5GHz RSSI for a BSSID to be considered */
	u16 min_2g_rssi;                           /* minimum 2.4GHz RSSI for a BSSID to be considered */
	u16 initial_score_max;                     /* maximum score that a network can have before bonuses */
	u8 current_connection_bonus;               /* report current connection bonus only, when there is a
						    * network's score this much higher than the current connection
						    */
	u8 same_network_bonus;                     /* score bonus for all networks with the same network flag */
	u8 secure_bonus;                           /* score bonus for networks that are not open */
	u8 band_5g_bonus;                          /* 5GHz RSSI score bonus (applied to all 5GHz networks) */
	u8 num_networks;                           /* number of wifi_epno_network objects */
	struct slsi_epno_ssid_param epno_ssid[];   /* PNO networks */
};

struct slsi_epno_hs2_param {
	u32 id;                          /* identifier of this network block, report this in event */
	u8  realm[256];                  /* null terminated UTF8 encoded realm, 0 if unspecified */
	s64 roaming_consortium_ids[16];  /* roaming consortium ids to match, 0s if unspecified */
	u8  plmn[3];                     /* mcc/mnc combination as per rules, 0s if unspecified */
};

struct slsi_rssi_monitor_evt {
	s16 rssi;
	u8  bssid[ETH_ALEN];
};

/* channel information */
struct slsi_lls_channel_info {
	enum slsi_lls_channel_width width;   /* channel width (20, 40, 80, 80+80, 160)*/
	int center_freq;   /* primary 20 MHz channel */
	int center_freq0;  /* center frequency (MHz) first segment */
	int center_freq1;  /* center frequency (MHz) second segment */
};

/* channel statistics */
struct slsi_lls_channel_stat {
	struct slsi_lls_channel_info channel;
	u32 on_time;                /* msecs the radio is awake (32 bits number accruing over time) */
	u32 cca_busy_time;          /* msecs the CCA register is busy (32 bits number accruing over time) */
};

/* wifi rate */
struct slsi_lls_rate {
	u32 preamble   :3;   /* 0: OFDM, 1:CCK, 2:HT 3:VHT 4..7 reserved*/
	u32 nss        :2;   /* 0:1x1, 1:2x2, 3:3x3, 4:4x4*/
	u32 bw         :3;   /* 0:20MHz, 1:40Mhz, 2:80Mhz, 3:160Mhz*/
	u32 rate_mcs_idx :8; /* OFDM/CCK rate code mcs index*/
	u32 reserved  :16;   /* reserved*/
	u32 bitrate;         /* units of 100 Kbps*/
};

/* per rate statistics */
struct slsi_lls_rate_stat {
	struct slsi_lls_rate rate;     /* rate information*/
	u32 tx_mpdu;        /* number of successfully transmitted data pkts (ACK rcvd)*/
	u32 rx_mpdu;        /* number of received data pkts*/
	u32 mpdu_lost;      /* number of data packet losses (no ACK)*/
	u32 retries;        /* total number of data pkt retries*/
	u32 retries_short;  /* number of short data pkt retries*/
	u32 retries_long;   /* number of long data pkt retries*/
};

/* radio statistics */
struct slsi_lls_radio_stat {
	int radio;               /* wifi radio (if multiple radio supported)*/
	u32 on_time;                    /* msecs the radio is awake (32 bits number accruing over time)*/
	u32 tx_time;                    /* msecs the radio is transmitting (32 bits number accruing over time)*/
	u32 rx_time;                    /* msecs the radio is in active receive (32 bits number accruing over time)*/
	u32 on_time_scan;               /* msecs the radio is awake due to all scan (32 bits number accruing over time)*/
	u32 on_time_nbd;                /* msecs the radio is awake due to NAN (32 bits number accruing over time)*/
	u32 on_time_gscan;              /* msecs the radio is awake due to G?scan (32 bits number accruing over time)*/
	u32 on_time_roam_scan;          /* msecs the radio is awake due to roam?scan (32 bits number accruing over time)*/
	u32 on_time_pno_scan;           /* msecs the radio is awake due to PNO scan (32 bits number accruing over time)*/
	u32 on_time_hs20;               /* msecs the radio is awake due to HS2.0 scans and GAS exchange (32 bits number accruing over time)*/
	u32 num_channels;               /* number of channels*/
	struct slsi_lls_channel_stat channels[];   /* channel statistics*/
};

struct slsi_lls_interface_link_layer_info {
	enum slsi_lls_interface_mode mode;     /* interface mode*/
	u8   mac_addr[6];                  /* interface mac address (self)*/
	enum slsi_lls_connection_state state;  /* connection state (valid for STA, CLI only)*/
	enum slsi_lls_roam_state roaming;      /* roaming state*/
	u32 capabilities;                  /* WIFI_CAPABILITY_XXX (self)*/
	u8 ssid[33];                       /* null terminated SSID*/
	u8 bssid[6];                       /* bssid*/
	u8 ap_country_str[3];              /* country string advertised by AP*/
	u8 country_str[3];                 /* country string for this association*/
};

/* per peer statistics */
struct slsi_lls_peer_info {
	enum slsi_lls_peer_type type;         /* peer type (AP, TDLS, GO etc.)*/
	u8 peer_mac_address[6];           /* mac address*/
	u32 capabilities;                 /* peer WIFI_CAPABILITY_XXX*/
	u32 num_rate;                     /* number of rates*/
	struct slsi_lls_rate_stat rate_stats[]; /* per rate statistics, number of entries  = num_rate*/
};

/* Per access category statistics */
struct slsi_lls_wmm_ac_stat {
	enum slsi_lls_traffic_ac ac;      /* access category (VI, VO, BE, BK)*/
	u32 tx_mpdu;                  /* number of successfully transmitted unicast data pkts (ACK rcvd)*/
	u32 rx_mpdu;                  /* number of received unicast data packets*/
	u32 tx_mcast;                 /* number of successfully transmitted multicast data packets*/
	u32 rx_mcast;                 /* number of received multicast data packets*/
	u32 rx_ampdu;                 /* number of received unicast a-mpdus; support of this counter is optional*/
	u32 tx_ampdu;                 /* number of transmitted unicast a-mpdus; support of this counter is optional*/
	u32 mpdu_lost;                /* number of data pkt losses (no ACK)*/
	u32 retries;                  /* total number of data pkt retries*/
	u32 retries_short;            /* number of short data pkt retries*/
	u32 retries_long;             /* number of long data pkt retries*/
	u32 contention_time_min;      /* data pkt min contention time (usecs)*/
	u32 contention_time_max;      /* data pkt max contention time (usecs)*/
	u32 contention_time_avg;      /* data pkt avg contention time (usecs)*/
	u32 contention_num_samples;   /* num of data pkts used for contention statistics*/
};

struct slsi_rx_data_cnt_details {
	int rx_unicast_cnt;     /*Total rx unicast packet which woke up host */
	int rx_multicast_cnt;   /*Total rx multicast packet which woke up host */
	int rx_broadcast_cnt;   /*Total rx broadcast packet which woke up host */
};

struct slsi_rx_wake_pkt_type_classification {
	int icmp_pkt;   /*wake icmp packet count */
	int icmp6_pkt;  /*wake icmp6 packet count */
	int icmp6_ra;   /*wake icmp6 RA packet count */
	int icmp6_na;   /*wake icmp6 NA packet count */
	int icmp6_ns;   /*wake icmp6 NS packet count */
};

struct slsi_rx_multicast_cnt {
	int ipv4_rx_multicast_addr_cnt; /*Rx wake packet was ipv4 multicast */
	int ipv6_rx_multicast_addr_cnt; /*Rx wake packet was ipv6 multicast */
	int other_rx_multicast_addr_cnt;/*Rx wake packet was non-ipv4 and non-ipv6*/
};

/*
 * Structure holding all the driver/firmware wake count reasons.
 *
 * Buffers for the array fields (cmd_event_wake_cnt/driver_fw_local_wake_cnt)
 * are allocated and freed by the framework. The size of each allocated
 * array is indicated by the corresponding |_cnt| field. HAL needs to fill in
 * the corresponding |_used| field to indicate the number of elements used in
 * the array.
 */
struct slsi_wlan_driver_wake_reason_cnt {
	int total_cmd_event_wake;    /* Total count of cmd event wakes */
	int *cmd_event_wake_cnt;     /* Individual wake count array, each index a reason */
	int cmd_event_wake_cnt_sz;   /* Max number of cmd event wake reasons */
	int cmd_event_wake_cnt_used; /* Number of cmd event wake reasons specific to the driver */

	int total_driver_fw_local_wake;    /* Total count of drive/fw wakes, for local reasons */
	int *driver_fw_local_wake_cnt;     /* Individual wake count array, each index a reason */
	int driver_fw_local_wake_cnt_sz;   /* Max number of local driver/fw wake reasons */
	int driver_fw_local_wake_cnt_used; /* Number of local driver/fw wake reasons specific to the driver */

	int total_rx_data_wake;     /* total data rx packets, that woke up host */
	struct slsi_rx_data_cnt_details rx_wake_details;
	struct slsi_rx_wake_pkt_type_classification rx_wake_pkt_classification_info;
	struct slsi_rx_multicast_cnt rx_multicast_wake_pkt_info;
};

/* interface statistics */
struct slsi_lls_iface_stat {
	void *iface;                          /* wifi interface*/
	struct slsi_lls_interface_link_layer_info info;  /* current state of the interface*/
	u32 beacon_rx;                        /* access point beacon received count from connected AP*/
	u64 average_tsf_offset;               /* average beacon offset encountered (beacon_TSF - TBTT)*/
	u32 leaky_ap_detected;                /* indicate that this AP typically leaks packets beyond the driver guard time.*/
	u32 leaky_ap_avg_num_frames_leaked;   /* average number of frame leaked by AP after frame with PM bit set was ACK'ed by AP*/
	u32 leaky_ap_guard_time;
	u32 mgmt_rx;                          /* access point mgmt frames received count from connected AP (including Beacon)*/
	u32 mgmt_action_rx;                   /* action frames received count*/
	u32 mgmt_action_tx;                   /* action frames transmit count*/
	int rssi_mgmt;                        /* access Point Beacon and Management frames RSSI (averaged)*/
	int rssi_data;                        /* access Point Data Frames RSSI (averaged) from connected AP*/
	int rssi_ack;                         /* access Point ACK RSSI (averaged) from connected AP*/
	struct slsi_lls_wmm_ac_stat ac[SLSI_LLS_AC_MAX];     /* per ac data packet statistics*/
	u32 num_peers;                        /* number of peers*/
	struct slsi_lls_peer_info peer_info[]; /* per peer statistics*/
};

#define SLSI_FAPI_NAN_CONFIG_PARAM_SID_BEACON 0X0003
#define SLSI_FAPI_NAN_CONFIG_PARAM_2_4_RSSI_CLOSE 0X0004
#define SLSI_FAPI_NAN_CONFIG_PARAM_2_4_RSSI_MIDDLE 0X0005
#define SLSI_FAPI_NAN_CONFIG_PARAM_2_4_RSSI_PROXIMITY 0X0006
#define SLSI_FAPI_NAN_CONFIG_PARAM_BAND_USAGE 0X0007
#define SLSI_FAPI_NAN_CONFIG_PARAM_5_RSSI_CLOSE 0X0008
#define SLSI_FAPI_NAN_CONFIG_PARAM_5_RSSI_MIDDLE 0X0009
#define SLSI_FAPI_NAN_CONFIG_PARAM_5_RSSI_PROXIMITY 0X000A
#define SLSI_FAPI_NAN_CONFIG_PARAM_HOP_COUNT_LIMIT 0X000B
#define SLSI_FAPI_NAN_CONFIG_PARAM_RSSI_WINDOW_SIZE 0X000C
#define SLSI_FAPI_NAN_CONFIG_PARAM_SCAN_PARAMETER_2_4 0X000D
#define SLSI_FAPI_NAN_CONFIG_PARAM_SCAN_PARAMETER_5 0X000E
#define SLSI_FAPI_NAN_CONFIG_PARAM_MASTER_PREFERENCE 0X000F
#define SLSI_FAPI_NAN_CONFIG_PARAM_CONNECTION_CAPAB 0X0010
#define SLSI_FAPI_NAN_CONFIG_PARAM_POST_DISCOVER_PARAM 0X0011
#define SLSI_FAPI_NAN_CONFIG_PARAM_FURTHER_AVAIL_CHANNEL_MAP 0X0012
#define SLSI_FAPI_NAN_CONFIG_PARAM_ADDR_RANDOM_INTERVAL 0X0013
#define SLSI_FAPI_NAN_SERVICE_NAME 0X0020
#define SLSI_FAPI_NAN_SERVICE_SPECIFIC_INFO 0X0021
#define SLSI_FAPI_NAN_RX_MATCH_FILTER 0X0022
#define SLSI_FAPI_NAN_TX_MATCH_FILTER 0X0023
#define SLSI_FAPI_NAN_SDF_MATCH_FILTER 0X0024
#define SLSI_FAPI_NAN_CLUSTER_ATTRIBUTE 0X0025

#define SLSI_HAL_NAN_MAX_SOCIAL_CHANNELS 3
#define SLSI_HAL_NAN_MAX_SERVICE_NAME_LEN 255
#define SLSI_HAL_NAN_MAX_SERVICE_SPECIFIC_INFO_LEN 1024
#define SLSI_HAL_NAN_MAX_MATCH_FILTER_LEN 255
#define SLSI_HAL_NAN_MAX_SUBSCRIBE_MAX_ADDRESS 42
#define SLSI_HAL_NAN_MAX_POSTDISCOVERY_LEN 5

enum slsi_wifi_hal_api_return_types {
	WIFI_HAL_SUCCESS = 0,
	WIFI_HAL_ERROR_NONE = 0,
	WIFI_HAL_ERROR_UNKNOWN = -1,
	WIFI_HAL_ERROR_UNINITIALIZED = -2,
	WIFI_HAL_ERROR_NOT_SUPPORTED = -3,
	WIFI_HAL_ERROR_NOT_AVAILABLE = -4,
	WIFI_HAL_ERROR_INVALID_ARGS = -5,
	WIFI_HAL_ERROR_INVALID_REQUEST_ID = -6,
	WIFI_HAL_ERROR_TIMED_OUT = -7,
	WIFI_HAL_ERROR_TOO_MANY_REQUESTS = -8,
	WIFI_HAL_ERROR_OUT_OF_MEMORY = -9
};

enum slsi_nan_status_type {
	/* NAN Protocol Response Codes */
	NAN_STATUS_SUCCESS = 0,
	NAN_STATUS_TIMEOUT = 1,
	NAN_STATUS_DE_FAILURE = 2,
	NAN_STATUS_INVALID_MSG_VERSION = 3,
	NAN_STATUS_INVALID_MSG_LEN = 4,
	NAN_STATUS_INVALID_MSG_ID = 5,
	NAN_STATUS_INVALID_HANDLE = 6,
	NAN_STATUS_NO_SPACE_AVAILABLE = 7,
	NAN_STATUS_INVALID_PUBLISH_TYPE = 8,
	NAN_STATUS_INVALID_TX_TYPE = 9,
	NAN_STATUS_INVALID_MATCH_ALGORITHM = 10,
	NAN_STATUS_DISABLE_IN_PROGRESS = 11,
	NAN_STATUS_INVALID_TLV_LEN = 12,
	NAN_STATUS_INVALID_TLV_TYPE = 13,
	NAN_STATUS_MISSING_TLV_TYPE = 14,
	NAN_STATUS_INVALID_TOTAL_TLVS_LEN = 15,
	NAN_STATUS_INVALID_MATCH_HANDLE = 16,
	NAN_STATUS_INVALID_TLV_VALUE = 17,
	NAN_STATUS_INVALID_TX_PRIORITY = 18,
	NAN_STATUS_INVALID_CONNECTION_MAP = 19,
	NAN_STATUS_INVALID_TCA_ID = 20,
	NAN_STATUS_INVALID_STATS_ID = 21,
	NAN_STATUS_NAN_NOT_ALLOWED = 22,
	NAN_STATUS_NO_OTA_ACK = 23,
	NAN_STATUS_TX_FAIL = 24,
	/* 25-4095 Reserved */
	/* NAN Configuration Response codes */
	NAN_STATUS_INVALID_RSSI_CLOSE_VALUE = 4096,
	NAN_STATUS_INVALID_RSSI_MIDDLE_VALUE = 4097,
	NAN_STATUS_INVALID_HOP_COUNT_LIMIT = 4098,
	NAN_STATUS_INVALID_MASTER_PREFERENCE_VALUE = 4099,
	NAN_STATUS_INVALID_LOW_CLUSTER_ID_VALUE = 4100,
	NAN_STATUS_INVALID_HIGH_CLUSTER_ID_VALUE = 4101,
	NAN_STATUS_INVALID_BACKGROUND_SCAN_PERIOD = 4102,
	NAN_STATUS_INVALID_RSSI_PROXIMITY_VALUE = 4103,
	NAN_STATUS_INVALID_SCAN_CHANNEL = 4104,
	NAN_STATUS_INVALID_POST_NAN_CONNECTIVITY_CAPABILITIES_BITMAP = 4105,
	NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_NUMCHAN_VALUE = 4106,
	NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_DURATION_VALUE = 4107,
	NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_CLASS_VALUE = 4108,
	NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_CHANNEL_VALUE = 4109,
	NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_AVAILABILITY_INTERVAL_BITMAP_VALUE = 4110,
	NAN_STATUS_INVALID_FURTHER_AVAILABILITY_MAP_MAP_ID = 4111,
	NAN_STATUS_INVALID_POST_NAN_DISCOVERY_CONN_TYPE_VALUE = 4112,
	NAN_STATUS_INVALID_POST_NAN_DISCOVERY_DEVICE_ROLE_VALUE = 4113,
	NAN_STATUS_INVALID_POST_NAN_DISCOVERY_DURATION_VALUE = 4114,
	NAN_STATUS_INVALID_POST_NAN_DISCOVERY_BITMAP_VALUE = 4115,
	NAN_STATUS_MISSING_FUTHER_AVAILABILITY_MAP = 4116,
	NAN_STATUS_INVALID_BAND_CONFIG_FLAGS = 4117,
	NAN_STATUS_INVALID_RANDOM_FACTOR_UPDATE_TIME_VALUE = 4118,
	NAN_STATUS_INVALID_ONGOING_SCAN_PERIOD = 4119,
	NAN_STATUS_INVALID_DW_INTERVAL_VALUE = 4120,
	NAN_STATUS_INVALID_DB_INTERVAL_VALUE = 4121,
	/* 4122-8191 RESERVED */
	NAN_TERMINATED_REASON_INVALID = 8192,
	NAN_TERMINATED_REASON_TIMEOUT = 8193,
	NAN_TERMINATED_REASON_USER_REQUEST = 8194,
	NAN_TERMINATED_REASON_FAILURE = 8195,
	NAN_TERMINATED_REASON_COUNT_REACHED = 8196,
	NAN_TERMINATED_REASON_DE_SHUTDOWN = 8197,
	NAN_TERMINATED_REASON_DISABLE_IN_PROGRESS = 8198,
	NAN_TERMINATED_REASON_POST_DISC_ATTR_EXPIRED = 8199,
	NAN_TERMINATED_REASON_POST_DISC_LEN_EXCEEDED = 8200,
	NAN_TERMINATED_REASON_FURTHER_AVAIL_MAP_EMPTY = 8201
};

enum slsi_nan_response_type {
	NAN_RESPONSE_ENABLED                = 0,
	NAN_RESPONSE_DISABLED               = 1,
	NAN_RESPONSE_PUBLISH                = 2,
	NAN_RESPONSE_PUBLISH_CANCEL         = 3,
	NAN_RESPONSE_TRANSMIT_FOLLOWUP      = 4,
	NAN_RESPONSE_SUBSCRIBE              = 5,
	NAN_RESPONSE_SUBSCRIBE_CANCEL       = 6,
	NAN_RESPONSE_STATS                  = 7,
	NAN_RESPONSE_CONFIG                 = 8,
	NAN_RESPONSE_TCA                    = 9,
	NAN_RESPONSE_ERROR                  = 10,
	NAN_RESPONSE_BEACON_SDF_PAYLOAD     = 11,
	NAN_RESPONSE_GET_CAPABILITIES       = 12
};

enum slsi_nan_disc_event_type {
	NAN_EVENT_ID_DISC_MAC_ADDR = 0,
	NAN_EVENT_ID_STARTED_CLUSTER,
	NAN_EVENT_ID_JOINED_CLUSTER
};

struct slsi_hal_nan_social_channel_scan_params {
	u8 dwell_time[SLSI_HAL_NAN_MAX_SOCIAL_CHANNELS];
	u16 scan_period[SLSI_HAL_NAN_MAX_SOCIAL_CHANNELS];
};

struct slsi_hal_nan_connectivity_capability {
	u8 payload_transmit_flag;
	u8 is_wfd_supported;
	u8 is_wfds_supported;
	u8 is_tdls_supported;
	u8 is_ibss_supported;
	u8 is_mesh_supported;
	u8 wlan_infra_field;
};

struct slsi_hal_nan_post_discovery_param {
	u8 type; /* NanConnectionType */
	u8 role; /* NanDeviceRole */
	u8 transmit_freq;
	u8 duration; /* NanAvailDuration */
	u32 avail_interval_bitmap;
	u8 addr[ETH_ALEN];
	u16 mesh_id_len;
	u8 mesh_id[32];
	u16 infrastructure_ssid_len;
	u8 infrastructure_ssid_val[32];
};

struct slsi_hal_nan_further_availability_channel {
	/* struct slsi_hal_nan_further_availability_channel*/
	u8 entry_control;
	u8 class_val;
	u8 channel;
	u8 mapid;
	u32 avail_interval_bitmap;
};

struct slsi_hal_nan_further_availability_map {
	u8 numchans;
	struct slsi_hal_nan_further_availability_channel famchan[32];
};

struct slsi_hal_nan_receive_post_discovery {
	u8 type;
	u8 role;
	u8 duration;
	u32 avail_interval_bitmap;
	u8 mapid;
	u8 addr[ETH_ALEN];
	u16 mesh_id_len;
	u8 mesh_id[32];
	u16 infrastructure_ssid_len;
	u8 infrastructure_ssid_val[32];
};

struct slsi_hal_nan_enable_req {
	/* Mandatory parameters below */
	u8 master_pref;
	u16 cluster_low;
	u16 cluster_high;

	u8 config_support_5g;
	u8 support_5g_val;
	u8 config_sid_beacon;
	u8 sid_beacon_val;
	u8 config_2dot4g_rssi_close;
	u8 rssi_close_2dot4g_val;

	u8 config_2dot4g_rssi_middle;
	u8 rssi_middle_2dot4g_val;

	u8 config_2dot4g_rssi_proximity;
	u8 rssi_proximity_2dot4g_val;

	u8 config_hop_count_limit;
	u8 hop_count_limit_val;

	u8 config_2dot4g_support;
	u8 support_2dot4g_val;

	u8 config_2dot4g_beacons;
	u8 beacon_2dot4g_val;
	u8 config_2dot4g_sdf;
	u8 sdf_2dot4g_val;
	u8 config_5g_beacons;
	u8 beacon_5g_val;
	u8 config_5g_sdf;
	u8 sdf_5g_val;
	u8 config_5g_rssi_close;
	u8 rssi_close_5g_val;
	u8 config_5g_rssi_middle;
	u8 rssi_middle_5g_val;
	u8 config_5g_rssi_close_proximity;
	u8 rssi_close_proximity_5g_val;
	u8 config_rssi_window_size;
	u8 rssi_window_size_val;
	/* The 24 bit Organizationally Unique ID + the 8 bit Network Id. */
	u8 config_oui;
	u32 oui_val;
	u8 config_intf_addr;
	u8 intf_addr_val[ETH_ALEN];

	u8 config_cluster_attribute_val;
	u8 config_scan_params;
	struct slsi_hal_nan_social_channel_scan_params scan_params_val;
	u8 config_random_factor_force;
	u8 random_factor_force_val;
	u8 config_hop_count_force;
	u8 hop_count_force_val;

	/* channel frequency in MHz to enable Nan on */
	u8 config_24g_channel;
	u32 channel_24g_val;

	u8 config_5g_channel;
	int channel_5g_val;
};

struct slsi_hal_nan_publish_req {
	/* id  0 means new publish, any other id is existing publish */
	u16 publish_id;
	/* how many seconds to run for. 0 means forever until canceled */
	u16 ttl;
	/* periodicity of OTA unsolicited publish.
	 * Specified in increments of 500 ms
	 */
	u16 period;
	u8 publish_type;/* 0= unsolicited, solicited = 1, 2= both */
	u8 tx_type; /* 0 = broadcast, 1= unicast  if solicited publish */
	/* number of OTA Publish, 0 means forever until canceled */
	u8 publish_count;
	u16 service_name_len;
	u8 service_name[SLSI_HAL_NAN_MAX_SERVICE_NAME_LEN];
	u8 publish_match_indicator;

	u16 service_specific_info_len;
	u8 service_specific_info[SLSI_HAL_NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];

	u16 rx_match_filter_len;
	u8 rx_match_filter[SLSI_HAL_NAN_MAX_MATCH_FILTER_LEN];

	u16 tx_match_filter_len;
	u8 tx_match_filter[SLSI_HAL_NAN_MAX_MATCH_FILTER_LEN];

	u8 rssi_threshold_flag;

	/* 8-bit bitmap which allows the Host to associate this publish
	 *  with a particular Post-NAN Connectivity attribute
	 *  which has been sent down in a NanConfigureRequest/NanEnableRequest
	 *  message.  If the DE fails to find a configured Post-NAN
	 * connectivity attributes referenced by the bitmap,
	 *  the DE will return an error code to the Host.
	 *  If the Publish is configured to use a Post-NAN Connectivity
	 *  attribute and the Host does not refresh the Post-NAN Connectivity
	 *  attribute the Publish will be canceled and the Host will be sent
	 *  a PublishTerminatedIndication message.
	 */
	u8 connmap;
	/* Set/Enable corresponding bits to disable any
	 * indications that follow a publish.
	 * BIT0 - Disable publish termination indication.
	 * BIT1 - Disable match expired indication.
	 * BIT2 - Disable followUp indication received (OTA).
	 */
	u8 recv_indication_cfg;
};

struct slsi_hal_nan_subscribe_req {
	/* id 0 means new subscribe, non zero is existing subscribe */
	u16 subscribe_id;
	/* how many seconds to run for. 0 means forever until canceled */
	u16 ttl;
	/* periodicity of OTA Active Subscribe. Units in increments
	 * of 500 ms , 0 = attempt every DW
	 */
	u16 period;

	/* Flag which specifies how the Subscribe request shall be processed. */
	u8 subscribe_type; /* 0 - PASSIVE , 1- ACTIVE */

	/* Flag which specifies on Active Subscribes how the Service Response
	 * Filter attribute is populated.
	 */
	u8 service_response_filter; /* 0 - Bloom Filter, 1 - MAC Addr */

	/* Flag which specifies how the Service Response Filter Include
	 * bit is populated.
	 * 0=Do not respond if in the Address Set, 1= Respond
	 */
	u8 service_response_include;

	/* Flag which specifies if the Service Response Filter
	 * should be used when creating Subscribes.
	 * 0=Do not send the Service Response Filter,1= send
	 */
	u8 use_service_response_filter;

	/* Flag which specifies if the Service Specific Info is needed in
	 *  the Publish message before creating the MatchIndication
	 */
	u8 ssi_required_for_match_indication; /* 0=Not needed, 1= Required */

	/* Field which specifies how matching indication to host is controlled.
	 *  0 - Match and Indicate Once
	 *  1 - Match and Indicate continuous
	 *  2 - Match and Indicate never. This means don't
	 *      indicate match to host.
	 *  3 - Reserved
	 */
	u8 subscribe_match_indicator;

	/* The number of Subscribe Matches which should occur
	 *  before the Subscribe request is automatically terminated.
	 */
	/* If this value is 0 this field is not used by DE.*/
	u8 subscribe_count;

	/* length of service name */
	/* UTF-8 encoded string identifying the service */
	u16 service_name_len;
	u8 service_name[SLSI_HAL_NAN_MAX_SERVICE_NAME_LEN];

	/* Sequence of values which further specify the published service
	 * beyond the service name
	 */
	u16 service_specific_info_len;
	u8 service_specific_info[SLSI_HAL_NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];

	/* Ordered sequence of <length, value> pairs used to filter out
	 * received publish discovery messages.
	 *  This can be sent both for a Passive or an Active Subscribe
	 */
	u16 rx_match_filter_len;
	u8 rx_match_filter[SLSI_HAL_NAN_MAX_MATCH_FILTER_LEN];

	/* Ordered sequence of <length, value> pairs  included in the
	 *  Discovery Frame when an Active Subscribe is used.
	 */
	u16 tx_match_filter_len;
	u8 tx_match_filter[SLSI_HAL_NAN_MAX_MATCH_FILTER_LEN];
	u8 rssi_threshold_flag;

	u8 connmap;
	/* NAN Interface Address, conforming to the format as described in
	 *  8.2.4.3.2 of IEEE Std. 802.11-2012.
	 */
	u8 num_intf_addr_present;
	u8 intf_addr[SLSI_HAL_NAN_MAX_SUBSCRIBE_MAX_ADDRESS][ETH_ALEN];
	/* Set/Enable corresponding bits to disable
	 * indications that follow a subscribe.
	 * BIT0 - Disable subscribe termination indication.
	 * BIT1 - Disable match expired indication.
	 * BIT2 - Disable followUp indication received (OTA).
	 */
	u8 recv_indication_cfg;
};

struct slsi_hal_nan_transmit_followup_req {
	/* Publish or Subscribe Id of an earlier Publish/Subscribe */
	u16 publish_subscribe_id;

	/* This Id is the Requestor Instance that is passed as
	 *  part of earlier MatchInd/FollowupInd message.
	 */
	u32 requestor_instance_id;
	u8 addr[ETH_ALEN]; /* Unicast address */
	u8 priority; /* priority of the request 2=high */
	u8 dw_or_faw; /* 0= send in a DW, 1=send in FAW */

	/* Sequence of values which further specify the published service beyond
	 *  the service name.
	 */
	u16 service_specific_info_len;
	u8 service_specific_info[SLSI_HAL_NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];
	/* Set/Enable corresponding bits to disable
	 * responses after followUp.
	 * BIT0 - Disable followUp response from FW.
	 */
	u8 recv_indication_cfg;
};

struct slsi_hal_nan_config_req {
	u8 config_sid_beacon;
	u8 sid_beacon;
	u8 config_rssi_proximity;
	u8 rssi_proximity;
	u8 config_master_pref;
	u8 master_pref;
	/* 1 byte value which defines the RSSI filter threshold.
	 *  Any Service Descriptors received above this value
	 *  that are configured for RSSI filtering will be dropped.
	 *  The rssi values should be specified without sign.
	 *  For eg: -70dBm should be specified as 70.
	 */
	u8 config_5g_rssi_close_proximity;
	u8 rssi_close_proximity_5g_val;
	u8 config_rssi_window_size;
	u16 rssi_window_size_val;
	/* If set to 1, the Discovery Engine will enclose the Cluster
	 *  Attribute only sent in Beacons in a Vendor Specific Attribute
	 *  and transmit in a Service Descriptor Frame.
	 */
	u8 config_cluster_attribute_val;
	u8 config_scan_params;
	struct slsi_hal_nan_social_channel_scan_params scan_params_val;
	/* 1 byte quantity which forces the Random Factor to a particular
	 * value for all transmitted Sync/Discovery beacons
	 */
	u8 config_random_factor_force;
	u8 random_factor_force_val;
	/* 1 byte quantity which forces the HC for all transmitted Sync and
	 *  Discovery Beacon NO matter the real HC being received over the
	 *  air.
	 */
	u8 config_hop_count_force;
	u8 hop_count_force_val;
	/* NAN Post Connectivity Capability */
	u8 config_conn_capability;
	struct slsi_hal_nan_connectivity_capability conn_capability_val;
	/* NAN Post Discover Capability */
	u8 num_config_discovery_attr;
	struct slsi_hal_nan_post_discovery_param discovery_attr_val[SLSI_HAL_NAN_MAX_POSTDISCOVERY_LEN];
	/* NAN Further availability Map */
	u8 config_fam;
	struct slsi_hal_nan_further_availability_map fam_val;
};

struct slsi_hal_nan_capabilities {
	u32 max_concurrent_nan_clusters;
	u32 max_publishes;
	u32 max_subscribes;
	u32 max_service_name_len;
	u32 max_match_filter_len;
	u32 max_total_match_filter_len;
	u32 max_service_specific_info_len;
	u32 max_vsa_data_len;
	u32 max_mesh_data_len;
	u32 max_ndi_interfaces;
	u32 max_ndp_sessions;
	u32 max_app_info_len;
};

struct slsi_hal_nan_followup_ind {
	u16 publish_subscribe_id;
	u32 requestor_instance_id;
	u8 addr[ETH_ALEN];
	u8 dw_or_faw;
	u16 service_specific_info_len;
	u8 service_specific_info[SLSI_HAL_NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];
};

struct slsi_hal_nan_match_ind {
	u16 publish_subscribe_id;
	u32 requestor_instance_id;
	u8 addr[ETH_ALEN];
	u16 service_specific_info_len;
	u8 service_specific_info[SLSI_HAL_NAN_MAX_SERVICE_SPECIFIC_INFO_LEN];
	u16 sdf_match_filter_len;
	u8 sdf_match_filter[SLSI_HAL_NAN_MAX_MATCH_FILTER_LEN];
	u8 match_occurred_flag;
	u8 out_of_resource_flag;
	u8 rssi_value;
	u8 is_conn_capability_valid;
	struct slsi_hal_nan_connectivity_capability conn_capability;
	u8 num_rx_discovery_attr;
	struct slsi_hal_nan_receive_post_discovery discovery_attr[SLSI_HAL_NAN_MAX_POSTDISCOVERY_LEN];
	u8 num_chans;
	struct slsi_hal_nan_further_availability_channel famchan[32];
	u8 cluster_attribute_len;
	u8 cluster_attribute[32];
};

void slsi_nl80211_vendor_init(struct slsi_dev *sdev);
void slsi_nl80211_vendor_deinit(struct slsi_dev *sdev);
u8 slsi_gscan_get_scan_policy(enum wifi_band band);
void slsi_gscan_handle_scan_result(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb, u16 scan_id, bool scan_done);
int slsi_mlme_set_bssid_hotlist_req(struct slsi_dev *sdev, struct net_device *dev, struct slsi_nl_hotlist_param *nl_hotlist_param);
void slsi_hotlist_ap_lost_indication(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb);
void slsi_gscan_hash_remove(struct slsi_dev *sdev, u8 *mac);
void slsi_rx_significant_change_ind(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb);
int slsi_gscan_alloc_buckets(struct slsi_dev *sdev, struct slsi_gscan *gscan, int num_buckets);
int slsi_vendor_event(struct slsi_dev *sdev, int event_id, const void *data, int len);
int slsi_mib_get_gscan_cap(struct slsi_dev *sdev, struct slsi_nl_gscan_capabilities *cap);
void slsi_rx_rssi_report_ind(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb);

#ifdef CONFIG_SCSC_WLAN_ENHANCED_LOGGING
void slsi_rx_event_log_indication(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb);
#endif
void slsi_nan_event(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb);
void slsi_nan_followup_ind(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb);
void slsi_nan_service_ind(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb);

void slsi_check_num_radios(struct slsi_dev *sdev);


static inline bool slsi_is_gscan_id(u16 scan_id)
{
	if ((scan_id >= SLSI_GSCAN_SCAN_ID_START) && (scan_id <= SLSI_GSCAN_SCAN_ID_END))
		return true;

	return false;
}

static inline enum slsi_lls_traffic_ac slsi_fapi_to_android_traffic_q(enum slsi_traffic_q fapi_q)
{
	switch (fapi_q) {
	case SLSI_TRAFFIC_Q_BE:
		return SLSI_LLS_AC_BE;
	case SLSI_TRAFFIC_Q_BK:
		return SLSI_LLS_AC_BK;
	case SLSI_TRAFFIC_Q_VI:
		return SLSI_LLS_AC_VI;
	case SLSI_TRAFFIC_Q_VO:
		return SLSI_LLS_AC_VO;
	default:
		return SLSI_LLS_AC_MAX;
	}
}
#endif
