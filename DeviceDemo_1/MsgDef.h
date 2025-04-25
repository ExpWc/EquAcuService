
typedef unsigned int UINT32;
typedef unsigned long long ULL;

struct EmtStruct {
	UINT32 scnPlatId;			// 平台Id    4
	UINT32 scnEmtId;			// 辐射源Id
	UINT32 scnPlatForce;		// 平台敌我属性 0-无报告，1-敌方，2-我方，3-友方，4-中立，5-敌方盟国，6-敌方友邦，7-不明；
	UINT32 scnPlatType;			// 平台类型 0-未知，1-地面固定，2-地面移动，3-水面，4-水下，5-导弹，6-固定翼，7-旋转翼，8-无人机，9-卫星，10-升空平台，12-便携，13以后预留
	UINT32 scnEmtType;			// 辐射源 类型0-未知，1-警戒雷达，2-对空警戒雷达，3-对海警戒雷达，4-低空警戒雷达，5-引导雷达，6-目标指示雷达，
								//			 7-测高雷达，8-预警雷达，9-导弹预警雷达，10-武器控制雷达，11-炮瞄雷达，12-导弹制导雷达，13-导弹攻击雷达，
								//			 14-鱼雷攻击雷达，15-多功能雷达，16-导弹末制导雷达，17-轰炸瞄准雷达，18-侦察雷达，19-战场侦察雷达，
								//			 20-炮位侦察校射雷达，21-活动目标侦察校射雷达，22-航空管制雷达，23-导航雷达，24-地形跟随雷达，25-着陆引导雷达，
								//			 26-港口监视雷达，27-测量雷达，28-气象雷达，29-敌我识别器，30-机载雷达，31-成像雷达
	UINT32 scnTgtPos[2];		// 目标位置:经度 纬度 单位：10^-7度
	float scnTgtHight;			// 目标高度 单位：m
	float scnTgtAzim;			// 目标方位 单位：度
	float scnTgtElev;			// 目标俯仰 单位：度
	float scnTgtSdist;			// 目标斜距 单位：m
	float scnTgtRadSpd;			// 目标径向速度 正为接近，负为远离 单位：m/s
	float scnTgtTagSpd;			// 目标切向速度 正为逆时针，负为顺时针 单位：m/s
	float scnSgnPowCode;		// 信号功率码  dBm - 120 - 100
	UINT32 scnEmtWkMod;			// 辐射源工作模式 0-未知，1-搜索，2-跟踪，3-制导，4-TWS，5-TAS，6以后预留
	UINT32 scnEmtPlorMod;		// 辐射源极化方式 0-未知，1-左圆极化，2-右圆极化，3-垂直极化，4-水平极化，5-斜极化，6-极化可选择，7-极化编码，8-极化捷变，9-左旋椭圆极化，10-右旋椭圆极化，11以后预留
	UINT32 scnSgnFM;			// 信号调制方式 0-不明，1-常规，2-非线性调频，3-线性调频，4-二相编码，5-四相编码，6-频率编码，7-频率分集，8以后预留
	float scnAntScanProd;		// 天线扫描周期 单位：ms
	float scnBeamDwlTm;			// 波位驻留时间
	float scnIntfDur;			// 相干处理间隔
	UINT32 scnSgnRFTyp;			// 信号频率类型 0-不明，1-固定，2-组变，3-分集，4-编码，5-捷变，6-射频参差，7-连续波频率调制，8-连续波编码调制，9-连续波噪声调制，10-脉内频率编码，11以后预留
	UINT32 scnSgnRFCnt;			// 信号频点数
	float scnSgnRFMed;			// 信号频率中值 单位：MHz
	float scnSgnRFRng;			// 信号频率范围 单位：MHz
	float scnSgnRFList[64];		// 频点列表 单位：MHz
	UINT32 scnSgnPRITyp;		// 信号PRI类型 0-不明，1-固定，2-跳变，3-抖动，4-捷变，5-参差，6-编码，7-滑变，8-连续波，9以后预留
	UINT32 scnSgnPRICnt;		// 信号PRI点数
	float scnSgnPRIMed;			// 信号PRI中值 单位：us
	float scnSgnPRIRng;			// 信号PRI范围 单位：us
	float scnSgnPRIList[64];	// PRI列表 单位：us
	UINT32 scnSgnPWTyp;			// 信号PW类型 0-不明，1-固定，2-变化，3以后预留
	UINT32 scnSgnPWCnt;			// 信号PW点数
	float scnSgnPWMed;			// 信号PW中值 单位：us
	float scnSgnPWRange;		// 信号PW范围 单位：us
	float scnSgnPWList[8];		// 信号PW列表 单位：us
	UINT32 scnEmtWkModID;		// 辐射源工作模式ID
};

struct RFScnUpdataStruct
{
	UINT32 scnEquID;			//装备ID
	float scnEquCourse;			//侦察载机航向
	UINT32 scnTime[7];			//上报时间,年,月，日，时，分，秒,毫秒       7*4
	UINT32 scnEmtCount;			//辐射源个数
	EmtStruct etp2[32];			//辐射源结构体
};