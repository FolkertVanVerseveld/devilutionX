//HEADER_GOES_HERE
#ifndef __STORES_H__
#define __STORES_H__

extern int stextup;    // weak
extern int storenumh;  // weak
extern int stextlhold; // weak
extern ItemStruct boyitem;
extern int stextshold; // idb
extern ItemStruct premiumitem[6];
extern void *pSTextBoxCels;
extern int premiumlevel; // idb
extern int talker;       // weak
extern STextStruct stext[24];
extern char stextsize;  // weak
extern int stextsmax;   // weak
extern int InStoreFlag; // idb
extern ItemStruct storehold[48];
extern int gossipstart; // weak
extern ItemStruct witchitem[20];
extern int stextscrl;
extern int numpremium; // idb
extern ItemStruct healitem[20];
extern ItemStruct golditem;
extern char storehidx[48];
extern void *pSTextSlidCels;
extern int stextvhold;     // weak
extern int stextsel;       // weak
extern char stextscrldbtn; // weak
extern int gossipend;      // weak
extern void *pCelBuff;
extern int stextsval; // idb
extern int boylevel;  // weak
extern ItemStruct smithitem[20];
extern int stextdown;      // weak
extern char stextscrlubtn; // weak
extern char stextflag;

void InitStores();
void SetupTownStores();
void FreeStoreMem();
void DrawSTextBack();
void PrintSString(int x, int y, unsigned char cjustflag, char *str, int col, int val);
void DrawSLine(int y);
void DrawSArrows(int y1, int y2);
void DrawSTextHelp();
void ClearSText(int s, int e);
void AddSLine(int y);
void AddSTextVal(int y, int val);
void OffsetSTextY(int y, int yo);
void AddSText(int x, int y, int j, char *str, char clr, int sel);
void StoreAutoPlace();
void S_StartSmith();
void S_ScrollSBuy(int idx);
void PrintStoreItem(ItemStruct *x, int l, char iclr);
void S_StartSBuy();
void S_ScrollSPBuy(int idx);
BOOL S_StartSPBuy();
BOOL SmithSellOk(int i);
void S_ScrollSSell(int idx);
void S_StartSSell();
BOOL SmithRepairOk(int i);
void S_StartSRepair();
void AddStoreHoldRepair(ItemStruct *itm, int i);
void S_StartWitch();
void S_ScrollWBuy(int idx);
void S_StartWBuy();
BOOL WitchSellOk(int i);
void S_StartWSell();
BOOL WitchRechargeOk(int i);
void AddStoreHoldRecharge(ItemStruct itm, int i);
void S_StartWRecharge();
void S_StartNoMoney();
void S_StartNoRoom();
void S_StartConfirm();
void S_StartBoy();
void S_StartBBoy();
void S_StartHealer();
void S_ScrollHBuy(int idx);
void S_StartHBuy();
void S_StartStory();
BOOL IdItemOk(ItemStruct *i);
void AddStoreHoldId(ItemStruct itm, int i);
void S_StartSIdentify();
void S_StartIdShow();
void S_StartTalk();
void S_StartTavern();
void S_StartBarMaid();
void S_StartDrunk();
void StartStore(char s);
void DrawSText();
void STextESC();
void STextUp();
void STextDown();
void STextPrior();
void STextNext();
void S_SmithEnter();
void SetGoldCurs(int pnum, int i);
void SetSpdbarGoldCurs(int pnum, int i);
void TakePlrsMoney(int cost);
void SmithBuyItem();
void S_SBuyEnter();
void SmithBuyPItem();
void S_SPBuyEnter();
BOOL StoreGoldFit(int idx);
void PlaceStoreGold(int v);
void StoreSellItem();
void S_SSellEnter();
void SmithRepairItem();
void S_SRepairEnter();
void S_WitchEnter();
void WitchBuyItem();
void S_WBuyEnter();
void S_WSellEnter();
void WitchRechargeItem();
void S_WRechargeEnter();
void S_BoyEnter();
void BoyBuyItem();
void HealerBuyItem();
void S_BBuyEnter();
void StoryIdItem();
void S_ConfirmEnter();
void S_HealerEnter();
void S_HBuyEnter();
void S_StoryEnter();
void S_SIDEnter();
void S_TalkEnter();
void S_TavernEnter();
void S_BarmaidEnter();
void S_DrunkEnter();
void STextEnter();
void CheckStoreBtn();
void ReleaseStoreBtn();

/* rdata */

extern int SStringY[24];
extern char *talkname[9];

#endif /* __STORES_H__ */
