/******************************************************************************
 *
 * Copyright (c) 2012 - 2018 Samsung Electronics Co., Ltd. All rights reserved
 *
 *****************************************************************************/

#ifndef __SLSI_MGT_H__
#define __SLSI_MGT_H__

#include <linux/mutex.h>

#include "dev.h"
#include "debug.h"

/* For 3.4.11 kernel support */
#ifndef WLAN_OUI_MICROSOFT
#define WLAN_OUI_MICROSOFT              0x0050f2
#define WLAN_OUI_TYPE_MICROSOFT_WPA     1
#define WLAN_OUI_TYPE_MICROSOFT_WMM     2
#define WLAN_OUI_TYPE_MICROSOFT_WPS     4
#endif

#define SLSI_COUNTRY_CODE_LEN 3

#define SLSI_EAPOL_TYPE_RSN_KEY          (2)
#define SLSI_EAPOL_TYPE_WPA_KEY          (254)

#define SLSI_IEEE8021X_TYPE_EAPOL_KEY    3
#define SLSI_IEEE8021X_TYPE_EAP_PACKET   0

#define SLSI_EAPOL_KEY_INFO_KEY_TYPE_BIT_IN_LOWER_BYTE     (1 << 3) /* Group = 0, Pairwise = 1 */
#define SLSI_EAPOL_KEY_INFO_MIC_BIT_IN_HIGHER_BYTE           (1 << 0)
/* pkt_data would start from 802.1X Authentication field (pkt_data[0] = Version).
 * For M4 packet, it will be something as below... member(size, position)
 * Version (1, 0) + Type (1, 1) + Length (2, 2:3) + Descriptor Type (1, 4) + Key Information (2, 5:6) +
 *  key_length(2, 7:8) + replay_counter(8, 9:16) + key_nonce(32, 17:48) + key_iv(16, 49:64) +
 *  key_rsc (8, 65:72) + key_id(16, 73:80) + key_mic (16, 81:96) + key_data_length(2, 97:98) +
 *  keydata(key_data_length, 99:99+key_data_length)
 */
#define SLSI_EAPOL_IEEE8021X_TYPE_POS                       (1)
#define SLSI_EAPOL_TYPE_POS                                 (4)
#define SLSI_EAPOL_KEY_INFO_HIGHER_BYTE_POS                 (5)
#define SLSI_EAPOL_KEY_INFO_LOWER_BYTE_POS                  (6)
#define SLSI_EAPOL_KEY_DATA_LENGTH_HIGHER_BYTE_POS          (97)
#define SLSI_EAPOL_KEY_DATA_LENGTH_LOWER_BYTE_POS           (98)

#define SLSI_80211_AC_VO 0
#define SLSI_80211_AC_VI 1
#define SLSI_80211_AC_BE 2
#define SLSI_80211_AC_BK 3

/* IF Number (Index) based checks */
#define SLSI_IS_VIF_INDEX_WLAN(ndev_vif) (ndev_vif->ifnum == SLSI_NET_INDEX_WLAN)
#define SLSI_IS_VIF_INDEX_P2P(ndev_vif) (ndev_vif->ifnum == SLSI_NET_INDEX_P2P)
#ifdef CONFIG_SCSC_WLAN_WIFI_SHARING
#define SLSI_IS_VIF_INDEX_P2P_GROUP(ndev_vif) ((ndev_vif->ifnum == SLSI_NET_INDEX_P2PX_SWLAN) &&\
					       (ndev_vif->wifi_sharing == 0))
#define SLSI_IS_INTERFACE_WIFI_SHARING_AP(ndev_vif) ((ndev_vif->ifnum == SLSI_NET_INDEX_P2PX_SWLAN) &&\
						     (ndev_vif->wifi_sharing == 1))
#else
#define SLSI_IS_VIF_INDEX_P2P_GROUP(ndev_vif) (ndev_vif->ifnum == SLSI_NET_INDEX_P2PX_SWLAN)
#endif
#define SLSI_IS_VIF_INDEX_NAN(ndev_vif) ((ndev_vif)->ifnum == SLSI_NET_INDEX_NAN)

/* Check for P2P unsync vif type */
#define SLSI_IS_P2P_UNSYNC_VIF(ndev_vif) ((ndev_vif->ifnum == SLSI_NET_INDEX_P2P) && (ndev_vif->vif_type == FAPI_VIFTYPE_UNSYNCHRONISED))

/* Check for HS unsync vif type */
#define SLSI_IS_HS2_UNSYNC_VIF(ndev_vif) ((ndev_vif->ifnum == SLSI_NET_INDEX_WLAN) && (ndev_vif->vif_type == FAPI_VIFTYPE_UNSYNCHRONISED))

/* Check for P2P Group role */
#define SLSI_IS_P2P_GROUP_STATE(sdev)  ((sdev->p2p_state == P2P_GROUP_FORMED_GO) || (sdev->p2p_state == P2P_GROUP_FORMED_CLI))

/* Extra delay to wait after MLME-Roam.Response before obtaining roam reports */
#define SLSI_STA_ROAM_REPORT_EXTRA_DELAY_MSEC 50

/* Extra duration in addition to ROC duration - For any workqueue scheduling delay */
#define SLSI_P2P_ROC_EXTRA_MSEC 10

/* Extra duration to retain unsync vif even after ROC/mgmt_tx completes */
#define SLSI_P2P_UNSYNC_VIF_EXTRA_MSEC  1000

/* Extra duration to retain HS2 unsync vif even after mgmt_tx completes */
#define SLSI_HS2_UNSYNC_VIF_EXTRA_MSEC  1000

/* Increased wait duration to retain unsync vif for GO-Negotiated to complete
 * due to delayed response or, to allow peer to retry GO-Negotiation
 */
#define SLSI_P2P_NEG_PROC_UNSYNC_VIF_RETAIN_DURATION 3000

/* Extra duration in addition to mgmt tx wait */
#define SLSI_P2P_MGMT_TX_EXTRA_MSEC  100

#define SLSI_FORCE_SCHD_ACT_FRAME_MSEC 100
#define SLSI_P2PGO_KEEP_ALIVE_PERIOD_SEC 10
#define SLSI_P2PGC_CONN_TIMEOUT_MSEC 10000

/* P2P Public Action Frames */
#define SLSI_P2P_PA_GO_NEG_REQ  0
#define SLSI_P2P_PA_GO_NEG_RSP          1
#define SLSI_P2P_PA_GO_NEG_CFM  2
#define SLSI_P2P_PA_INV_REQ 3
#define SLSI_P2P_PA_INV_RSP 4
#define SLSI_P2P_PA_DEV_DISC_REQ        5
#define SLSI_P2P_PA_DEV_DISC_RSP        6
#define SLSI_P2P_PA_PROV_DISC_REQ       7
#define SLSI_P2P_PA_PROV_DISC_RSP       8
#define SLSI_P2P_PA_INVALID 0xFF

/* Service discovery public action frame types */
#define SLSI_PA_GAS_INITIAL_REQ  (10)
#define SLSI_PA_GAS_INITIAL_RSP  (11)
#define SLSI_PA_GAS_COMEBACK_REQ (12)
#define SLSI_PA_GAS_COMEBACK_RSP (13)

/* For service discovery action frames dummy subtype is used by setting the 7th bit */
#define SLSI_PA_GAS_DUMMY_SUBTYPE_MASK   0x80
#define SLSI_PA_GAS_INITIAL_REQ_SUBTYPE  (SLSI_PA_GAS_INITIAL_REQ | SLSI_PA_GAS_DUMMY_SUBTYPE_MASK)
#define SLSI_PA_GAS_INITIAL_RSP_SUBTYPE  (SLSI_PA_GAS_INITIAL_RSP | SLSI_PA_GAS_DUMMY_SUBTYPE_MASK)
#define SLSI_PA_GAS_COMEBACK_REQ_SUBTYPE (SLSI_PA_GAS_COMEBACK_REQ | SLSI_PA_GAS_DUMMY_SUBTYPE_MASK)
#define SLSI_PA_GAS_COMEBACK_RSP_SUBTYPE (SLSI_PA_GAS_COMEBACK_RSP | SLSI_PA_GAS_DUMMY_SUBTYPE_MASK)

#define SLSI_P2P_STATUS_ATTR_ID 0
#define SLSI_P2P_STATUS_CODE_SUCCESS 0

#define SLSI_ROAMING_CHANNEL_CACHE_TIMEOUT (5 * 60)

#define SLSI_RX_SEQ_NUM_MASK        0xFFF
#define SLSI_RX_VIA_TDLS_LINK       0x8000

#define SET_ETHERTYPE_PATTERN_DESC(pd, ethertype) \
	pd.offset  = 0x0C; \
	pd.mask_length = 2; \
	pd.mask[0] = 0xff; \
	pd.mask[1] = 0xff; \
	pd.pattern[0] = ethertype >> 8; \
	pd.pattern[1] = ethertype & 0xFF

/* For checking DHCP frame */
#define SLSI_IP_TYPE_UDP 0x11
#define SLSI_IP_TYPE_OFFSET 23
#define SLSI_IP_SOURCE_PORT_OFFSET 34
#define SLSI_IP_DEST_PORT_OFFSET 36
#define SLSI_DHCP_SERVER_PORT 67
#define SLSI_DHCP_CLIENT_PORT 68

#define SLSI_DHCP_MSG_MAGIC_OFFSET 278
#define SLSI_DHCP_OPTION 53
#define SLSI_DHCP_MESSAGE_TYPE_OFFER 0x02
#define SLSI_DHCP_MESSAGE_TYPE_ACK 0x05

enum slsi_dhcp_tx {
	SLSI_TX_IS_NOT_DHCP,
	SLSI_TX_IS_DHCP_SERVER,
	SLSI_TX_IS_DHCP_CLIENT
};

enum slsi_fw_regulatory_rule_flags {
	SLSI_REGULATORY_NO_IR = 1 << 0,
		SLSI_REGULATORY_DFS = 1 << 1,
		SLSI_REGULATORY_NO_OFDM = 1 << 2,
		SLSI_REGULATORY_NO_INDOOR = 1 << 3,
		SLSI_REGULATORY_NO_OUTDOOR = 1 << 4
};

enum slsi_sta_conn_state {
	SLSI_STA_CONN_STATE_DISCONNECTED = 0,
	SLSI_STA_CONN_STATE_CONNECTING = 1,
	SLSI_STA_CONN_STATE_DOING_KEY_CONFIG = 2,
	SLSI_STA_CONN_STATE_CONNECTED = 3
};

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 13, 0))
static inline unsigned compare_ether_addr(const u8 *addr1, const u8 *addr2)
{
	return !ether_addr_equal(addr1, addr2);
}
#endif

/**
 * Peer record handling:
 * Records are created/destroyed by the control path eg cfg80211 connect or
 * when handling a MLME-CONNECT-IND when the VIA is an AP.
 *
 * However peer records are also currently accessed from the data path in both
 * Tx and Rx directions:
 * Tx - to determine the queueset
 * Rx - for routing received packets back out to peers
 *
 * So the interactions required for the data path:
 * 1. can NOT block
 * 2. needs to be as quick as possible
 */
static inline struct slsi_peer *slsi_get_peer_from_mac(struct slsi_dev *sdev, struct net_device *dev, const u8 *mac)
{
	struct netdev_vif *ndev_vif = netdev_priv(dev);

	(void)sdev; /* unused */

	/* Accesses the peer records but doesn't block as called from the data path.
	 * MUST check the valid flag on the record before accessing any other data in the record.
	 * Records are static, so having obtained a pointer the pointer will remain valid
	 * it just maybe the data that it points to gets set to ZERO.
	 */

	if (ndev_vif->vif_type == FAPI_VIFTYPE_STATION) {
		if (ndev_vif->sta.tdls_enabled) {
			int i;

			for (i = 1; i < SLSI_TDLS_PEER_INDEX_MAX; i++)
				if (ndev_vif->peer_sta_record[i] && ndev_vif->peer_sta_record[i]->valid &&
				    compare_ether_addr(ndev_vif->peer_sta_record[i]->address, mac) == 0)
					return ndev_vif->peer_sta_record[i];
		}
		if (ndev_vif->peer_sta_record[SLSI_STA_PEER_QUEUESET] && ndev_vif->peer_sta_record[SLSI_STA_PEER_QUEUESET]->valid)
			return ndev_vif->peer_sta_record[SLSI_STA_PEER_QUEUESET];
	} else if (ndev_vif->vif_type == FAPI_VIFTYPE_AP) {
		int i = 0;

		for (i = 0; i < SLSI_PEER_INDEX_MAX; i++)
			if (ndev_vif->peer_sta_record[i] && ndev_vif->peer_sta_record[i]->valid &&
			    compare_ether_addr(ndev_vif->peer_sta_record[i]->address, mac) == 0)
				return ndev_vif->peer_sta_record[i];
	}
	return NULL;
}

static inline struct slsi_peer *slsi_get_peer_from_qs(struct slsi_dev *sdev, struct net_device *dev, u16 queueset)
{
	struct netdev_vif *ndev_vif = netdev_priv(dev);

	(void)sdev; /* unused */

	if (!ndev_vif->peer_sta_record[queueset] || !ndev_vif->peer_sta_record[queueset]->valid)
		return NULL;

	return ndev_vif->peer_sta_record[queueset];
}

static inline bool slsi_is_tdls_peer(struct net_device *dev, struct slsi_peer *peer)
{
	struct netdev_vif *ndev_vif = netdev_priv(dev);

	return (ndev_vif->vif_type == FAPI_VIFTYPE_STATION) && (peer->aid >= SLSI_TDLS_PEER_INDEX_MIN);
}

static inline bool slsi_is_proxy_arp_supported_on_ap(struct sk_buff *assoc_resp_ie)
{
	const u8 *ie = cfg80211_find_ie(WLAN_EID_EXT_CAPABILITY, assoc_resp_ie->data, assoc_resp_ie->len);

	if ((ie) && (ie[1] > 1))
		return ie[3] & 0x10;     /*0: eid, 1: len; 3: proxy arp is 12th bit*/

	return 0;
}

static inline int slsi_cache_ies(const u8 *src_ie, size_t src_ie_len, u8 **dest_ie, size_t *dest_ie_len)
{
	*dest_ie = kmalloc(src_ie_len, GFP_KERNEL);
	if (*dest_ie == NULL)
		return -ENOMEM;

	memcpy(*dest_ie, src_ie, src_ie_len);
	*dest_ie_len = src_ie_len;

	return 0;
}

static inline void slsi_clear_cached_ies(u8 **ie, size_t *ie_len)
{
	if (*ie_len != 0)
		kfree(*ie);
	*ie = NULL;
	*ie_len = 0;
}

/* P2P Public Action frame subtype in text format for debug purposes */
static inline char *slsi_p2p_pa_subtype_text(int subtype)
{
	switch (subtype) {
	case SLSI_P2P_PA_GO_NEG_REQ:
		return "GO_NEG_REQ";
	case SLSI_P2P_PA_GO_NEG_RSP:
		return "GO_NEG_RSP";
	case SLSI_P2P_PA_GO_NEG_CFM:
		return "GO_NEG_CFM";
	case SLSI_P2P_PA_INV_REQ:
		return "INV_REQ";
	case SLSI_P2P_PA_INV_RSP:
		return "INV_RSP";
	case SLSI_P2P_PA_DEV_DISC_REQ:
		return "DEV_DISC_REQ";
	case SLSI_P2P_PA_DEV_DISC_RSP:
		return "DEV_DISC_RSP";
	case SLSI_P2P_PA_PROV_DISC_REQ:
		return "PROV_DISC_REQ";
	case SLSI_P2P_PA_PROV_DISC_RSP:
		return "PROV_DISC_RSP";
	case SLSI_PA_GAS_INITIAL_REQ_SUBTYPE:
		return "GAS_INITIAL_REQUEST";
	case SLSI_PA_GAS_INITIAL_RSP_SUBTYPE:
		return "GAS_INITIAL_RESPONSE";
	case SLSI_PA_GAS_COMEBACK_REQ_SUBTYPE:
		return "GAS_COMEBACK_REQUEST";
	case SLSI_PA_GAS_COMEBACK_RSP_SUBTYPE:
		return "GAS_COMEBACK_RESPONSE";
	case SLSI_P2P_PA_INVALID:
		return "PA_INVALID";
	default:
		return "UNKNOWN";
	}
}

/* Cookie generation and assignment for user space ROC and mgmt_tx request from supplicant */
static inline void slsi_assign_cookie_id(u64 *cookie, u64 *counter)
{
	(*cookie) = ++(*counter);
	if ((*cookie) == 0)
		(*cookie) = ++(*counter);
}

/* Update P2P Probe Response IEs in driver */
static inline void slsi_unsync_vif_set_probe_rsp_ie(struct netdev_vif *ndev_vif, u8 *ies, size_t ies_len)
{
	if (ndev_vif->unsync.probe_rsp_ies_len)
		kfree(ndev_vif->unsync.probe_rsp_ies);
	ndev_vif->unsync.probe_rsp_ies = ies;
	ndev_vif->unsync.probe_rsp_ies_len = ies_len;
}

/* Set management frame tx data of vif */
static inline int slsi_set_mgmt_tx_data(struct netdev_vif *ndev_vif, u64 cookie, u16 host_tag, const u8 *buf, size_t buf_len)
{
	u8 *tx_frame = NULL;

	if (buf_len != 0) {
		tx_frame = kmalloc(buf_len, GFP_KERNEL);
		if (!tx_frame) {
			SLSI_NET_ERR(ndev_vif->wdev.netdev, "FAILED to allocate memory for Tx frame\n");
			return -ENOMEM;
		}
		SLSI_NET_DBG3(ndev_vif->wdev.netdev, SLSI_CFG80211, "Copy buffer for tx_status\n");
		memcpy(tx_frame, buf, buf_len);
	} else if (ndev_vif->mgmt_tx_data.buf) {
		SLSI_NET_DBG3(ndev_vif->wdev.netdev, SLSI_CFG80211, "Free buffer of tx_status\n");
		kfree(ndev_vif->mgmt_tx_data.buf);
	}

	ndev_vif->mgmt_tx_data.cookie = cookie;
	ndev_vif->mgmt_tx_data.host_tag = host_tag;
	ndev_vif->mgmt_tx_data.buf = tx_frame;
	ndev_vif->mgmt_tx_data.buf_len = buf_len;

	return 0;
}

/**
 * Handler to queue P2P unsync vif deletion work.
 */
static inline void slsi_p2p_queue_unsync_vif_del_work(struct netdev_vif *ndev_vif, unsigned int delay)
{
	cancel_delayed_work(&ndev_vif->unsync.del_vif_work);
	queue_delayed_work(ndev_vif->sdev->device_wq, &ndev_vif->unsync.del_vif_work, msecs_to_jiffies(delay));
}

/* Update the new state for P2P. Also log the state change for debug purpose */
#define SLSI_P2P_STATE_CHANGE(sdev, next_state) \
	do { \
		SLSI_DBG1(sdev, SLSI_CFG80211, "P2P state change: %s -> %s\n", slsi_p2p_state_text(sdev->p2p_state), slsi_p2p_state_text(next_state)); \
		sdev->p2p_state = next_state; \
	} while (0)

void slsi_purge_scan_results(struct netdev_vif *ndev_vif, u16 scan_id);
void slsi_purge_scan_results_locked(struct netdev_vif *ndev_vif, u16 scan_id);
struct sk_buff *slsi_dequeue_cached_scan_result(struct slsi_scan *scan, int *count);
void slsi_get_hw_mac_address(struct slsi_dev *sdev, u8 *addr);
int slsi_start(struct slsi_dev *sdev);
void slsi_stop_net_dev(struct slsi_dev *sdev, struct net_device *dev);
void slsi_stop(struct slsi_dev *sdev);
void slsi_stop_locked(struct slsi_dev *sdev);
struct slsi_peer *slsi_peer_add(struct slsi_dev *sdev, struct net_device *dev, u8 *peer_address, u16 aid);
void slsi_peer_update_assoc_req(struct slsi_dev *sdev, struct net_device *dev, struct slsi_peer *peer, struct sk_buff *skb);
void slsi_peer_update_assoc_rsp(struct slsi_dev *sdev, struct net_device *dev, struct slsi_peer *peer, struct sk_buff *skb);
void slsi_peer_reset_stats(struct slsi_dev *sdev, struct net_device *dev, struct slsi_peer *peer);
int slsi_peer_remove(struct slsi_dev *sdev, struct net_device *dev, struct slsi_peer *peer);
int slsi_ps_port_control(struct slsi_dev *sdev, struct net_device *dev, struct slsi_peer *peer, enum slsi_sta_conn_state s);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
int slsi_del_station(struct wiphy *wiphy, struct net_device *dev,
		     struct station_del_parameters *del_params);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
int slsi_del_station(struct wiphy *wiphy, struct net_device *dev, const u8 *mac);
#else
int slsi_del_station(struct wiphy *wiphy, struct net_device *dev, u8 *mac);
#endif

int slsi_vif_activated(struct slsi_dev *sdev, struct net_device *dev);
void slsi_vif_deactivated(struct slsi_dev *sdev, struct net_device *dev);
int slsi_handle_disconnect(struct slsi_dev *sdev, struct net_device *dev, u8 *peer_address, u16 reason);
int slsi_band_update(struct slsi_dev *sdev, int band);
int slsi_ip_address_changed(struct slsi_dev *sdev, struct net_device *dev, __be32 ipaddress);
int slsi_send_gratuitous_arp(struct slsi_dev *sdev, struct net_device *dev);
struct ieee80211_channel *slsi_find_scan_channel(struct slsi_dev *sdev, struct ieee80211_mgmt *mgmt, size_t mgmt_len, u16 freq);
int slsi_auto_chan_select_scan(struct slsi_dev *sdev, int chan_count, struct ieee80211_channel *channels[]);
int slsi_set_uint_mib(struct slsi_dev *dev, struct net_device *ndev, u16 psid, int value);
int slsi_update_regd_rules(struct slsi_dev *sdev, bool country_check);
int slsi_set_boost(struct slsi_dev *sdev, struct net_device *dev);
int slsi_p2p_init(struct slsi_dev *sdev, struct netdev_vif *ndev_vif);
void slsi_p2p_deinit(struct slsi_dev *sdev, struct netdev_vif *ndev_vif);
int slsi_p2p_vif_activate(struct slsi_dev *sdev, struct net_device *dev, struct ieee80211_channel *chan, u16 duration, bool set_probe_rsp_ies);
void slsi_p2p_vif_deactivate(struct slsi_dev *sdev, struct net_device *dev, bool hw_available);
void slsi_p2p_group_start_remove_unsync_vif(struct slsi_dev *sdev);
int slsi_p2p_dev_probe_rsp_ie(struct slsi_dev *sdev, struct net_device *dev, u8 *probe_rsp_ie, size_t probe_rsp_ie_len);
int slsi_p2p_dev_null_ies(struct slsi_dev *sdev, struct net_device *dev);
int slsi_p2p_get_public_action_subtype(const struct ieee80211_mgmt *mgmt);
int slsi_p2p_get_go_neg_rsp_status(struct net_device *dev, const struct ieee80211_mgmt *mgmt);
u8 slsi_p2p_get_exp_peer_frame_subtype(u8 subtype);
int slsi_send_txq_params(struct slsi_dev *sdev, struct net_device *ndev);
void slsi_abort_sta_scan(struct slsi_dev *sdev);
int slsi_is_dhcp_packet(u8 *data);
void slsi_set_packet_filters(struct slsi_dev *sdev, struct net_device *dev);
int  slsi_update_packet_filters(struct slsi_dev *sdev, struct net_device *dev);
int  slsi_clear_packet_filters(struct slsi_dev *sdev, struct net_device *dev);
int slsi_ap_prepare_add_info_ies(struct netdev_vif *ndev_vif, const u8 *ies, size_t ies_len);
int slsi_set_mib_roam(struct slsi_dev *dev, struct net_device *ndev, u16 psid, int value);
int slsi_set_mib_rssi_boost(struct slsi_dev *sdev, struct net_device *dev, u16 psid, int index, int boost);
void slsi_modify_ies_on_channel_switch(struct net_device *dev, struct cfg80211_ap_settings *settings,
				       u8 *ds_params_ie, u8 *ht_operation_ie, struct ieee80211_mgmt  *mgmt,
				       u16 beacon_ie_head_len);
#ifdef CONFIG_SCSC_WLAN_WIFI_SHARING
bool slsi_if_valid_wifi_sharing_channel(struct slsi_dev *sdev, int freq);
void slsi_extract_valid_wifi_sharing_channels(struct slsi_dev *sdev);
void slsi_select_wifi_sharing_ap_channel(struct wiphy *wiphy, struct net_device *dev,
					 struct cfg80211_ap_settings *settings, struct slsi_dev *sdev,
					 int *wifi_sharing_channel_switched);
int slsi_set_mib_wifi_sharing_5ghz_channel(struct slsi_dev *sdev, u16 psid, int value,
					   int offset, int readbyte, char *arg);
int slsi_get_byte_position(int bit);
int slsi_check_if_channel_restricted_already(struct slsi_dev *sdev, int channel);
#endif
struct net_device *slsi_new_interface_create(struct wiphy        *wiphy,
					     const char          *name,
					     enum nl80211_iftype type,
					     struct vif_params   *params);
int slsi_get_mib_roam(struct slsi_dev *sdev, u16 psid, int *mib_value);
void slsi_roam_channel_cache_add(struct slsi_dev *sdev, struct net_device *dev, struct sk_buff *skb);
void slsi_roam_channel_cache_prune(struct net_device *dev, int seconds);
int slsi_roaming_scan_configure_channels(struct slsi_dev *sdev, struct net_device *dev, const u8 *ssid, u8 *channels);
int slsi_send_max_transmit_msdu_lifetime(struct slsi_dev *dev, struct net_device *ndev, u32 msdu_lifetime);
int slsi_read_max_transmit_msdu_lifetime(struct slsi_dev *dev, struct net_device *ndev, u32 *msdu_lifetime);
int slsi_read_unifi_countrylist(struct slsi_dev *sdev, u16 psid);
int slsi_read_default_country(struct slsi_dev *sdev, u8 *alpha2, u16 index);
int slsi_read_disconnect_ind_timeout(struct slsi_dev *sdev, u16 psid);
int slsi_read_regulatory_rules(struct slsi_dev *sdev, struct slsi_802_11d_reg_domain *domain_info, const char *alpha2);
int slsi_set_country_update_regd(struct slsi_dev *sdev, const char *alpha2_code, int size);
void slsi_clear_offchannel_data(struct slsi_dev *sdev, bool acquire_lock);
int slsi_hs2_vif_activate(struct slsi_dev *sdev, struct net_device *dev, struct ieee80211_channel *chan, u16 duration);
void slsi_hs2_vif_deactivate(struct slsi_dev *sdev, struct net_device *devbool, bool hw_available);
int slsi_is_wes_action_frame(const struct ieee80211_mgmt *mgmt);
void slsi_scan_ind_timeout_handle(struct work_struct *work);
void slsi_vif_cleanup(struct slsi_dev *sdev, struct net_device *dev, bool hw_available);
void slsi_scan_cleanup(struct slsi_dev *sdev, struct net_device *dev);
void slsi_dump_stats(struct net_device *dev);
int slsi_send_hanged_vendor_event(struct slsi_dev *sdev, u16 scsc_panic_code);
void slsi_update_supported_channels_regd_flags(struct slsi_dev *sdev);
#ifdef CONFIG_SCSC_WLAN_HANG_TEST
int slsi_test_send_hanged_vendor_event(struct net_device *dev);
#endif
void slsi_hs2_dump_public_action_subtype(struct ieee80211_mgmt *mgmt, bool tx);
void slsi_reset_channel_flags(struct slsi_dev *sdev);
void slsi_del_ap_netif_work(struct work_struct *work);

/* Sysfs based mac address override */
void slsi_create_sysfs_macaddr(void);
void slsi_destroy_sysfs_macaddr(void);

#endif /*__SLSI_MGT_H__*/
