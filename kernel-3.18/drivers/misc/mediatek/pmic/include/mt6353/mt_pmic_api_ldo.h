
#ifndef _MT_PMIC_API_LDO_H_
#define _MT_PMIC_API_LDO_H_
/* -------- generated by version : ldo gen v1.0 ------------ */
/* -------- date : 2015-11-13                   ------------ */

/* -------- LDO VTCXO28 API ------------ LDO Type3 */
/* define EN by sw mode */
void pmic_ldo_vtcxo28_sw_en(int en_value);

enum ldo_vtcxo28_ensel {
	VTCXO28_ENSEL_SRCLKEN0,
	VTCXO28_ENSEL_SRCLKEN1,
	VTCXO28_ENSEL_SRCLKEN0_OR_SRCLKEN1,
	VTCXO28_ENSEL_SRCLKEN0_AND_SRCLKEN1,
	VTCXO28_ENSEL_SRCLKEN2,
	VTCXO28_ENSEL_SRCLKEN0_OR_SRCLKEN2,
	VTCXO28_ENSEL_SRCLKEN1_OR_SRCLKEN2,
	VTCXO28_ENSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define EN by hw mode */
void pmic_ldo_vtcxo28_hw_en(int ensel_value);

/* define EN STB by sw mode */
void pmic_ldo_vtcxo28_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vtcxo28_sw_lp_mode(int mode_value);

enum ldo_vtcxo28_lpsel {
	VTCXO28_LPSEL_SRCLKEN0,
	VTCXO28_LPSEL_SRCLKEN1,
	VTCXO28_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VTCXO28_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VTCXO28_LPSEL_SRCLKEN2,
	VTCXO28_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VTCXO28_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VTCXO28_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vtcxo28_hw_lp_mode(int mode_sel_value);
/* -------- LDO VTCXO28 API end ------------ /LDO Type3 */

/* -------- LDO VTCXO24 API ------------ LDO Type4 */
void pmic_ldo_vtcxo24_sw_en(int en_value);/*  api skip : (8)ALLEN+ALLLP */

enum ldo_vtcxo24_ensel {
	VTCXO24_ENSEL_SRCLKEN0,
	VTCXO24_ENSEL_SRCLKEN1,
	VTCXO24_ENSEL_SRCLKEN0_OR_SRCLKEN1,
	VTCXO24_ENSEL_SRCLKEN0_AND_SRCLKEN1,
	VTCXO24_ENSEL_SRCLKEN2,
	VTCXO24_ENSEL_SRCLKEN0_OR_SRCLKEN2,
	VTCXO24_ENSEL_SRCLKEN1_OR_SRCLKEN2,
	VTCXO24_ENSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define EN STB by sw mode */
void pmic_ldo_vtcxo24_sw_stb(int sel_value);

/* define EN by hw mode */
void pmic_ldo_vtcxo24_hw_en(int ensel_value);

/* define LP by sw mode */
void pmic_ldo_vtcxo24_sw_lp_mode(int mode_value);

/* define LP MODE by hw mode */
void pmic_ldo_vtcxo24_hw_lp_mode(int mode_sel_value);

enum ldo_vtcxo24_lpsel {
	VTCXO24_LPSEL_SRCLKEN0,
	VTCXO24_LPSEL_SRCLKEN1,
	VTCXO24_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VTCXO24_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VTCXO24_LPSEL_SRCLKEN2,
	VTCXO24_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VTCXO24_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VTCXO24_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* -------- LDO VTCXO24 API end ------------ /LDO Type4 */

/* -------- LDO VXO22  API ------------ LDO Type4 */
/* define EN by sw mode */
void pmic_ldo_vxo22_sw_en(int en_value);

enum ldo_vxo22_ensel {
	VXO22_ENSEL_SRCLKEN0,
	VXO22_ENSEL_SRCLKEN1,
	VXO22_ENSEL_SRCLKEN0_OR_SRCLKEN1,
	VXO22_ENSEL_SRCLKEN0_AND_SRCLKEN1,
	VXO22_ENSEL_SRCLKEN2,
	VXO22_ENSEL_SRCLKEN0_OR_SRCLKEN2,
	VXO22_ENSEL_SRCLKEN1_OR_SRCLKEN2,
	VXO22_ENSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define EN by hw mode */
void pmic_ldo_vxo22_hw_en(int ensel_value);

/* define EN STB by sw mode */
void pmic_ldo_vxo22_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vxo22_sw_lp_mode(int mode_value);

enum ldo_vxo22_lpsel {
	VXO22_LPSEL_SRCLKEN0,
	VXO22_LPSEL_SRCLKEN1,
	VXO22_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VXO22_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VXO22_LPSEL_SRCLKEN2,
	VXO22_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VXO22_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VXO22_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vxo22_hw_lp_mode(int mode_sel_value);
/* -------- LDO VXO22  API end ------------ /LDO Type4 */

/* -------- LDO VRF18  API ------------ LDO Type3 */
/* define EN by sw mode */
void pmic_ldo_vrf18_sw_en(int en_value);

enum ldo_vrf18_ensel {
	VRF18_ENSEL_SRCLKEN0,
	VRF18_ENSEL_SRCLKEN1,
	VRF18_ENSEL_SRCLKEN0_OR_SRCLKEN1,
	VRF18_ENSEL_SRCLKEN0_AND_SRCLKEN1,
	VRF18_ENSEL_SRCLKEN2,
	VRF18_ENSEL_SRCLKEN0_OR_SRCLKEN2,
	VRF18_ENSEL_SRCLKEN1_OR_SRCLKEN2,
	VRF18_ENSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define EN by hw mode */
void pmic_ldo_vrf18_hw_en(int ensel_value);

/* define EN STB by sw mode */
void pmic_ldo_vrf18_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vrf18_sw_lp_mode(int mode_value);

enum ldo_vrf18_lpsel {
	VRF18_LPSEL_SRCLKEN0,
	VRF18_LPSEL_SRCLKEN1,
	VRF18_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VRF18_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VRF18_LPSEL_SRCLKEN2,
	VRF18_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VRF18_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VRF18_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vrf18_hw_lp_mode(int mode_sel_value);
/* -------- LDO VRF18  API end ------------ /LDO Type3 */

/* -------- LDO VRF12  API ------------ LDO Type3 */
/* define EN by sw mode */
void pmic_ldo_vrf12_sw_en(int en_value);

enum ldo_vrf12_ensel {
	VRF12_ENSEL_SRCLKEN0,
	VRF12_ENSEL_SRCLKEN1,
	VRF12_ENSEL_SRCLKEN0_OR_SRCLKEN1,
	VRF12_ENSEL_SRCLKEN0_AND_SRCLKEN1,
	VRF12_ENSEL_SRCLKEN2,
	VRF12_ENSEL_SRCLKEN0_OR_SRCLKEN2,
	VRF12_ENSEL_SRCLKEN1_OR_SRCLKEN2,
	VRF12_ENSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define EN by hw mode */
void pmic_ldo_vrf12_hw_en(int ensel_value);

/* define EN STB by sw mode */
void pmic_ldo_vrf12_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vrf12_sw_lp_mode(int mode_value);

enum ldo_vrf12_lpsel {
	VRF12_LPSEL_SRCLKEN0,
	VRF12_LPSEL_SRCLKEN1,
	VRF12_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VRF12_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VRF12_LPSEL_SRCLKEN2,
	VRF12_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VRF12_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VRF12_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vrf12_hw_lp_mode(int mode_sel_value);

/* define FAST_TRAN_DL_EN by sw mode */
void pmic_ldo_vrf12_sw_fs_trans_dl(int en_value);

/* define FAST_TRAN_CL_EN by sw mode */
void pmic_ldo_vrf12_sw_fs_trans_cl(int en_value);

/* define FAST_TRAN_DL_EN by hw mode */
void pmic_ldo_vrf12_hw_fs_trans_dl(int ensel_value);

/* define FAST_TRAN_CL_EN by hw mode */
void pmic_ldo_vrf12_hw_fs_trans_cl(int ensel_value);
/* -------- LDO VRF12  API end ------------ /LDO Type3 */

/* -------- LDO VCN33  API ------------ LDO Type3 */
/* define EN by sw mode */
void pmic_ldo_vcn33_sw_en(int en_value);

void pmic_ldo_vcn33_bt_sw_en(int en_value);/*  api skip : (3)ALLEN */

void pmic_ldo_vcn33_wifi_sw_en(int en_value);/*  api skip : (3)ALLEN */


/* define EN STB by sw mode */
void pmic_ldo_vcn33_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vcn33_sw_lp_mode(int mode_value);

/* define LP MODE by hw mode */
void pmic_ldo_vcn33_hw_lp_mode(int mode_sel_value);

enum ldo_vcn33_ensel {
	VCN33_ENSEL_SRCLKEN0,
	VCN33_ENSEL_SRCLKEN1,
	VCN33_ENSEL_SRCLKEN0_OR_SRCLKEN1,
	VCN33_ENSEL_SRCLKEN0_AND_SRCLKEN1,
	VCN33_ENSEL_SRCLKEN2,
	VCN33_ENSEL_SRCLKEN0_OR_SRCLKEN2,
	VCN33_ENSEL_SRCLKEN1_OR_SRCLKEN2,
	VCN33_ENSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

void pmic_ldo_vcn33_hw_en(int ensel_value);
/* -------- LDO VCN33  API end ------------ /LDO Type3 */

/* -------- LDO VCN28  API ------------ LDO Type3 */
/* define EN by sw mode */
void pmic_ldo_vcn28_sw_en(int en_value);

enum ldo_vcn28_ensel {
	VCN28_ENSEL_SRCLKEN0,
	VCN28_ENSEL_SRCLKEN1,
	VCN28_ENSEL_SRCLKEN0_OR_SRCLKEN1,
	VCN28_ENSEL_SRCLKEN0_AND_SRCLKEN1,
	VCN28_ENSEL_SRCLKEN2,
	VCN28_ENSEL_SRCLKEN0_OR_SRCLKEN2,
	VCN28_ENSEL_SRCLKEN1_OR_SRCLKEN2,
	VCN28_ENSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define EN by hw mode */
void pmic_ldo_vcn28_hw_en(int ensel_value);

/* define EN STB by sw mode */
void pmic_ldo_vcn28_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vcn28_sw_lp_mode(int mode_value);

enum ldo_vcn28_lpsel {
	VCN28_LPSEL_SRCLKEN0,
	VCN28_LPSEL_SRCLKEN1,
	VCN28_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VCN28_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VCN28_LPSEL_SRCLKEN2,
	VCN28_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VCN28_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VCN28_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vcn28_hw_lp_mode(int mode_sel_value);
/* -------- LDO VCN28  API end ------------ /LDO Type3 */

/* -------- LDO VCN18  API ------------ LDO Type3 */
/* define EN by sw mode */
void pmic_ldo_vcn18_sw_en(int en_value);

enum ldo_vcn18_ensel {
	VCN18_ENSEL_SRCLKEN0,
	VCN18_ENSEL_SRCLKEN1,
	VCN18_ENSEL_SRCLKEN0_OR_SRCLKEN1,
	VCN18_ENSEL_SRCLKEN0_AND_SRCLKEN1,
	VCN18_ENSEL_SRCLKEN2,
	VCN18_ENSEL_SRCLKEN0_OR_SRCLKEN2,
	VCN18_ENSEL_SRCLKEN1_OR_SRCLKEN2,
	VCN18_ENSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define EN by hw mode */
void pmic_ldo_vcn18_hw_en(int ensel_value);

/* define EN STB by sw mode */
void pmic_ldo_vcn18_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vcn18_sw_lp_mode(int mode_value);

enum ldo_vcn18_lpsel {
	VCN18_LPSEL_SRCLKEN0,
	VCN18_LPSEL_SRCLKEN1,
	VCN18_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VCN18_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VCN18_LPSEL_SRCLKEN2,
	VCN18_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VCN18_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VCN18_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vcn18_hw_lp_mode(int mode_sel_value);
/* -------- LDO VCN18  API end ------------ /LDO Type3 */

/* -------- LDO VCAMA  API ------------ LDO Type0 */
/* define EN by sw mode */
void pmic_ldo_vcama_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vcama_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vcama_sw_lp_mode(int mode_value);
/* -------- LDO VCAMA  API end ------------ /LDO Type0 */

/* -------- LDO VCAMIO API ------------ LDO Type0 */
/* define EN by sw mode */
void pmic_ldo_vcamio_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vcamio_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vcamio_sw_lp_mode(int mode_value);
/* -------- LDO VCAMIO API end ------------ /LDO Type0 */

/* -------- LDO VLDO28 API ------------ LDO Type1 */
/* define EN by sw mode */
void pmic_ldo_vldo28_sw_en(int en_value);

void pmic_ldo_vldo28_0_sw_en(int en_value);/*  api skip : (3)ALLEN */

void pmic_ldo_vldo28_1_sw_en(int en_value);/*  api skip : (3)ALLEN */

/* define EN STB by sw mode */
void pmic_ldo_vldo28_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vldo28_sw_lp_mode(int mode_value);

/* define LP MODE by hw mode */
void pmic_ldo_vldo28_hw_lp_mode(int mode_sel_value);

/* define FAST_TRAN_DL_EN by sw mode */
void pmic_ldo_vldo28_sw_fs_trans_dl(int en_value);

/* define FAST_TRAN_CL_EN by sw mode */
void pmic_ldo_vldo28_sw_fs_trans_cl(int en_value);

/* define FAST_TRAN_DL_EN by hw mode */
void pmic_ldo_vldo28_hw_fs_trans_dl(int ensel_value);

/* define FAST_TRAN_CL_EN by hw mode */
void pmic_ldo_vldo28_hw_fs_trans_cl(int ensel_value);
/* -------- LDO VLDO28 API end ------------ /LDO Type1 */

/* -------- LDO VCAMD  API ------------ LDO Type0 */
/* define EN by sw mode */
void pmic_ldo_vcamd_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vcamd_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vcamd_sw_lp_mode(int mode_value);
/* -------- LDO VCAMD  API end ------------ /LDO Type0 */

/* -------- LDO VAUX18 API ------------ LDO Type4 */
/* define EN by sw mode */
void pmic_ldo_vaux18_sw_en(int en_value);

enum ldo_vaux18_ensel {
	VAUX18_ENSEL_SRCLKEN0,
	VAUX18_ENSEL_SRCLKEN1,
	VAUX18_ENSEL_SRCLKEN0_OR_SRCLKEN1,
	VAUX18_ENSEL_SRCLKEN0_AND_SRCLKEN1,
	VAUX18_ENSEL_SRCLKEN2,
	VAUX18_ENSEL_SRCLKEN0_OR_SRCLKEN2,
	VAUX18_ENSEL_SRCLKEN1_OR_SRCLKEN2,
	VAUX18_ENSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define EN by hw mode */
void pmic_ldo_vaux18_hw_en(int ensel_value);

/* define EN STB by sw mode */
void pmic_ldo_vaux18_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vaux18_sw_lp_mode(int mode_value);

enum ldo_vaux18_lpsel {
	VAUX18_LPSEL_SRCLKEN0,
	VAUX18_LPSEL_SRCLKEN1,
	VAUX18_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VAUX18_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VAUX18_LPSEL_SRCLKEN2,
	VAUX18_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VAUX18_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VAUX18_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vaux18_hw_lp_mode(int mode_sel_value);
/* -------- LDO VAUX18 API end ------------ /LDO Type4 */

/* -------- LDO VAUD28 API ------------ LDO Type4 */
/* define EN by sw mode */
void pmic_ldo_vaud28_sw_en(int en_value);

enum ldo_vaud28_ensel {
	VAUD28_ENSEL_SRCLKEN0,
	VAUD28_ENSEL_SRCLKEN1,
	VAUD28_ENSEL_SRCLKEN0_OR_SRCLKEN1,
	VAUD28_ENSEL_SRCLKEN0_AND_SRCLKEN1,
	VAUD28_ENSEL_SRCLKEN2,
	VAUD28_ENSEL_SRCLKEN0_OR_SRCLKEN2,
	VAUD28_ENSEL_SRCLKEN1_OR_SRCLKEN2,
	VAUD28_ENSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define EN by hw mode */
void pmic_ldo_vaud28_hw_en(int ensel_value);

/* define EN STB by sw mode */
void pmic_ldo_vaud28_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vaud28_sw_lp_mode(int mode_value);

enum ldo_vaud28_lpsel {
	VAUD28_LPSEL_SRCLKEN0,
	VAUD28_LPSEL_SRCLKEN1,
	VAUD28_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VAUD28_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VAUD28_LPSEL_SRCLKEN2,
	VAUD28_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VAUD28_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VAUD28_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vaud28_hw_lp_mode(int mode_sel_value);
/* -------- LDO VAUD28 API end ------------ /LDO Type4 */

/* -------- LDO VSRAM_PROC API ------------ LDO Type4 */
/* define EN by sw mode */
/* void pmic_ldo_vsram_proc_sw_en(int en_value); */

enum ldo_vsram_proc_ensel {
	VSRAM_PROC_ENSEL_SRCLKEN0,
	VSRAM_PROC_ENSEL_SRCLKEN1,
	VSRAM_PROC_ENSEL_SRCLKEN0_OR_SRCLKEN1,
	VSRAM_PROC_ENSEL_SRCLKEN0_AND_SRCLKEN1,
	VSRAM_PROC_ENSEL_SRCLKEN2,
	VSRAM_PROC_ENSEL_SRCLKEN0_OR_SRCLKEN2,
	VSRAM_PROC_ENSEL_SRCLKEN1_OR_SRCLKEN2,
	VSRAM_PROC_ENSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define EN by hw mode */
/* void pmic_ldo_vsram_proc_hw_en(int ensel_value); */

/* define EN STB by sw mode */
void pmic_ldo_vsram_proc_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vsram_proc_sw_lp_mode(int mode_value);

enum ldo_vsram_proc_lpsel {
	VSRAM_PROC_LPSEL_SRCLKEN0,
	VSRAM_PROC_LPSEL_SRCLKEN1,
	VSRAM_PROC_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VSRAM_PROC_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VSRAM_PROC_LPSEL_SRCLKEN2,
	VSRAM_PROC_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VSRAM_PROC_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VSRAM_PROC_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vsram_proc_hw_lp_mode(int mode_sel_value);
/* -------- LDO VSRAM_PROC API end ------------ /LDO Type4 */

/* -------- LDO VDRAM  API ------------ LDO Type5 */
/* define EN by sw mode */
void pmic_ldo_vdram_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vdram_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vdram_sw_lp_mode(int mode_value);

enum ldo_vdram_lpsel {
	VDRAM_LPSEL_SRCLKEN0,
	VDRAM_LPSEL_SRCLKEN1,
	VDRAM_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VDRAM_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VDRAM_LPSEL_SRCLKEN2,
	VDRAM_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VDRAM_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VDRAM_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vdram_hw_lp_mode(int mode_sel_value);

/* define FAST_TRAN_DL_EN by sw mode */
void pmic_ldo_vdram_sw_fs_trans_dl(int en_value);

/* define FAST_TRAN_CL_EN by sw mode */
void pmic_ldo_vdram_sw_fs_trans_cl(int en_value);

/* define FAST_TRAN_DL_EN by hw mode */
void pmic_ldo_vdram_hw_fs_trans_dl(int ensel_value);

/* define FAST_TRAN_CL_EN by hw mode */
void pmic_ldo_vdram_hw_fs_trans_cl(int ensel_value);
/* -------- LDO VDRAM  API end ------------ /LDO Type5 */

/* -------- LDO VSIM1  API ------------ LDO Type1 */
/* define EN by sw mode */
void pmic_ldo_vsim1_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vsim1_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vsim1_sw_lp_mode(int mode_value);

enum ldo_vsim1_lpsel {
	VSIM1_LPSEL_SRCLKEN0,
	VSIM1_LPSEL_SRCLKEN1,
	VSIM1_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VSIM1_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VSIM1_LPSEL_SRCLKEN2,
	VSIM1_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VSIM1_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VSIM1_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vsim1_hw_lp_mode(int mode_sel_value);
/* -------- LDO VSIM1  API end ------------ /LDO Type1 */

/* -------- LDO VSIM2  API ------------ LDO Type1 */
/* define EN by sw mode */
void pmic_ldo_vsim2_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vsim2_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vsim2_sw_lp_mode(int mode_value);

enum ldo_vsim2_lpsel {
	VSIM2_LPSEL_SRCLKEN0,
	VSIM2_LPSEL_SRCLKEN1,
	VSIM2_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VSIM2_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VSIM2_LPSEL_SRCLKEN2,
	VSIM2_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VSIM2_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VSIM2_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vsim2_hw_lp_mode(int mode_sel_value);
/* -------- LDO VSIM2  API end ------------ /LDO Type1 */

/* -------- LDO VIO28  API ------------ LDO Type5 */
/* define EN by sw mode */
void pmic_ldo_vio28_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vio28_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vio28_sw_lp_mode(int mode_value);

enum ldo_vio28_lpsel {
	VIO28_LPSEL_SRCLKEN0,
	VIO28_LPSEL_SRCLKEN1,
	VIO28_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VIO28_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VIO28_LPSEL_SRCLKEN2,
	VIO28_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VIO28_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VIO28_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vio28_hw_lp_mode(int mode_sel_value);
/* -------- LDO VIO28  API end ------------ /LDO Type5 */

/* -------- LDO VMC    API ------------ LDO Type5 */
/* define EN by sw mode */
void pmic_ldo_vmc_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vmc_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vmc_sw_lp_mode(int mode_value);

enum ldo_vmc_lpsel {
	VMC_LPSEL_SRCLKEN0,
	VMC_LPSEL_SRCLKEN1,
	VMC_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VMC_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VMC_LPSEL_SRCLKEN2,
	VMC_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VMC_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VMC_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vmc_hw_lp_mode(int mode_sel_value);

/* define FAST_TRAN_DL_EN by sw mode */
void pmic_ldo_vmc_sw_fs_trans_dl(int en_value);

/* define FAST_TRAN_CL_EN by sw mode */
void pmic_ldo_vmc_sw_fs_trans_cl(int en_value);

/* define FAST_TRAN_DL_EN by hw mode */
void pmic_ldo_vmc_hw_fs_trans_dl(int ensel_value);

/* define FAST_TRAN_CL_EN by hw mode */
void pmic_ldo_vmc_hw_fs_trans_cl(int ensel_value);
/* -------- LDO VMC    API end ------------ /LDO Type5 */

/* -------- LDO VMCH   API ------------ LDO Type5 */
/* define EN by sw mode */
void pmic_ldo_vmch_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vmch_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vmch_sw_lp_mode(int mode_value);

enum ldo_vmch_lpsel {
	VMCH_LPSEL_SRCLKEN0,
	VMCH_LPSEL_SRCLKEN1,
	VMCH_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VMCH_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VMCH_LPSEL_SRCLKEN2,
	VMCH_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VMCH_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VMCH_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vmch_hw_lp_mode(int mode_sel_value);

/* define FAST_TRAN_DL_EN by sw mode */
void pmic_ldo_vmch_sw_fs_trans_dl(int en_value);

/* define FAST_TRAN_CL_EN by sw mode */
void pmic_ldo_vmch_sw_fs_trans_cl(int en_value);

/* define FAST_TRAN_DL_EN by hw mode */
void pmic_ldo_vmch_hw_fs_trans_dl(int ensel_value);

/* define FAST_TRAN_CL_EN by hw mode */
void pmic_ldo_vmch_hw_fs_trans_cl(int ensel_value);
/* -------- LDO VMCH   API end ------------ /LDO Type5 */

/* -------- LDO VUSB33 API ------------ LDO Type5 */
/* define EN by sw mode */
void pmic_ldo_vusb33_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vusb33_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vusb33_sw_lp_mode(int mode_value);

enum ldo_vusb33_lpsel {
	VUSB33_LPSEL_SRCLKEN0,
	VUSB33_LPSEL_SRCLKEN1,
	VUSB33_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VUSB33_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VUSB33_LPSEL_SRCLKEN2,
	VUSB33_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VUSB33_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VUSB33_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vusb33_hw_lp_mode(int mode_sel_value);
/* -------- LDO VUSB33 API end ------------ /LDO Type5 */

/* -------- LDO VEMC33 API ------------ LDO Type5 */
/* define EN by sw mode */
void pmic_ldo_vemc33_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vemc33_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vemc33_sw_lp_mode(int mode_value);

enum ldo_vemc33_lpsel {
	VEMC33_LPSEL_SRCLKEN0,
	VEMC33_LPSEL_SRCLKEN1,
	VEMC33_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VEMC33_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VEMC33_LPSEL_SRCLKEN2,
	VEMC33_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VEMC33_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VEMC33_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vemc33_hw_lp_mode(int mode_sel_value);

/* define FAST_TRAN_DL_EN by sw mode */
void pmic_ldo_vemc33_sw_fs_trans_dl(int en_value);

/* define FAST_TRAN_CL_EN by sw mode */
void pmic_ldo_vemc33_sw_fs_trans_cl(int en_value);

/* define FAST_TRAN_DL_EN by hw mode */
void pmic_ldo_vemc33_hw_fs_trans_dl(int ensel_value);

/* define FAST_TRAN_CL_EN by hw mode */
void pmic_ldo_vemc33_hw_fs_trans_cl(int ensel_value);
/* -------- LDO VEMC33 API end ------------ /LDO Type5 */

/* -------- LDO VIO18  API ------------ LDO Type5 */
/* define EN by sw mode */
void pmic_ldo_vio18_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vio18_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vio18_sw_lp_mode(int mode_value);

enum ldo_vio18_lpsel {
	VIO18_LPSEL_SRCLKEN0,
	VIO18_LPSEL_SRCLKEN1,
	VIO18_LPSEL_SRCLKEN0_OR_SRCLKEN1,
	VIO18_LPSEL_SRCLKEN0_AND_SRCLKEN1,
	VIO18_LPSEL_SRCLKEN2,
	VIO18_LPSEL_SRCLKEN0_OR_SRCLKEN2,
	VIO18_LPSEL_SRCLKEN1_OR_SRCLKEN2,
	VIO18_LPSEL_SRCLKEN0_OR_SRCLKEN1_OR_SRCLKEN2
};

/* define LP MODE by hw mode */
void pmic_ldo_vio18_hw_lp_mode(int mode_sel_value);
/* -------- LDO VIO18  API end ------------ /LDO Type5 */

/* -------- LDO VIBR   API ------------ LDO Type0 */
/* define EN by sw mode */
void pmic_ldo_vibr_sw_en(int en_value);

/* define EN STB by sw mode */
void pmic_ldo_vibr_sw_stb(int sel_value);

/* define LP by sw mode */
void pmic_ldo_vibr_sw_lp_mode(int mode_value);
/* -------- LDO VIBR   API end ------------ /LDO Type0 */

/* -------- LDO TREF   API ------------ LDO Type2 */
/* define EN by sw mode */
void pmic_ldo_tref_sw_en(int en_value);

/* define EN by hw mode */
void pmic_ldo_tref_hw_en(int ensel_value);
/* -------- LDO TREF   API end ------------ /LDO Type2 */

#endif				/* _MT_PMIC_API_LDO_H_ */