#include "music.h"

/*******************************
 * 高频优化音符频率定义
 * 所有音符比标准高一个八度
 *******************************/
// 第3八度
#define DO_3   131   // C3
#define RE_3   147   // D3
#define MI_3   165   // E3
#define FA_3   175   // F3
#define SOL_3  196   // G3
#define LA_3   220   // A3
#define SI_3   247   // B3

// 第4八度
#define DO_4   262      // C4
#define RE_4   294      // D4
#define MI_4   330      // E4
#define FA_4   349      // F4
#define SOL_4  392      // G4
#define LA_4   440      // A4
#define SI_4   494      // B4

#define DOS_4  278      // C#4
#define RES_4  311      // D#4
#define MIS_4  349      // E#4
#define FAS_4  370      // F#4
#define SOLS_4 415      // G#4
#define LAS_4  466      // A#4
#define SIS_4  523      // B#4

// 第5八度
#define DO_5   523      // C5
#define RE_5   587      // D5
#define MI_5   659      // E5
#define FA_5   698      // F5
#define SOL_5  784      // G5
#define LA_5   880      // A5
#define SI_5   988      // B5

#define DOS_5  554      // C#5
#define RES_5  622      // D#5
#define MIS_5  698      // E#5
#define FAS_5  740      // F#5
#define SOLS_5 831      // G#5
#define LAS_5  932      // A#5
#define SIS_5  1047     // B#5

// 第6八度
#define DO_6   1047  // C6
#define RE_6   1175  // D6
#define MI_6   1319  // E6
#define FA_6   1397  // F6
#define SOL_6  1568  // G6

#define REST    0    // 休止符

/*******************************
 * 节拍定义
 *******************************/
#define WHOLE_NOTE     2500    // 全音符 (比原版稍快)
#define HALF_NOTE      (WHOLE_NOTE/2)      // 二分音符
#define QUARTER_NOTE   (WHOLE_NOTE/4)      // 四分音符
#define EIGHTH_NOTE    (WHOLE_NOTE/8)      // 八分音符
#define SIXTEENTH_NOTE (WHOLE_NOTE/16)     // 十六分音符
#define THIRTYTWO_NOTE (WHOLE_NOTE/32)     // 三十二分音符

#define NOW_TIM_HZ                  (1000000)
#define MUSIC_Delay(x)              (usleep(x * 1000))

Buzzer buzzer;

void set_beep_hz(uint32_t hz)
{
    buzzer.set_duty_freq(50, hz);
}

void beep_stop(void)
{
    buzzer.set_duty(0);
}

// 播放单个音符
void play_note(uint32_t frequency, uint32_t duration) {
    if(frequency == REST) {
        beep_stop();
    } else {
        set_beep_hz(frequency);
    }
    MUSIC_Delay(duration);
    beep_stop();
    // 更短的音符间隔，使音乐更紧凑
    MUSIC_Delay(duration / 16);
}

// 播放音乐
void play_music(const Note *song, uint32_t length) {
    
    for(uint32_t i = 0; i < length; i++) {
        play_note(song[i].frequency, song[i].duration);
    }
}

/*******************************
 * 周杰伦《晴天》完整主旋律
 * 调式：G大调
 * 节拍：4/4拍
 *******************************/
const Note qing_tian[] = {

    // ===== 主歌第一段 =====
    // "故事的小黄花 从出生那年就飘着"
    {REST, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE},
    {SOL_4, QUARTER_NOTE}, {LA_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE},
    {REST, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE},
    {SOL_4, EIGHTH_NOTE}, {LA_4, SIXTEENTH_NOTE}, {SI_4, SIXTEENTH_NOTE}, 
    {LA_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {RE_4, EIGHTH_NOTE},
    
    // "童年的荡秋千 随记忆一直晃到现在"
    {REST, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE},
    {SOL_4, QUARTER_NOTE}, {LA_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE},
    {REST, EIGHTH_NOTE}, {SI_4, QUARTER_NOTE}, {LA_4, SIXTEENTH_NOTE}, {SI_4, SIXTEENTH_NOTE}, {DO_5, SIXTEENTH_NOTE}, {SI_4, SIXTEENTH_NOTE},
    {LA_4, SIXTEENTH_NOTE}, {DO_5, SIXTEENTH_NOTE}, {SI_4, SIXTEENTH_NOTE}, {LA_4, SIXTEENTH_NOTE}, {SOL_4, EIGHTH_NOTE},
    
    // ===== 主歌第二段 =====
    // "Re So So Si Do Si La So La"
    {RE_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE},
    {DO_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {LA_4, EIGHTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {LA_4, SIXTEENTH_NOTE},
    
    // "Si Si Si Si La Si La So"
    {SI_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE},
    {LA_4, SIXTEENTH_NOTE}, {SI_4, SIXTEENTH_NOTE}, {LA_4, EIGHTH_NOTE}, {SOL_4, QUARTER_NOTE},
    
    // "吹着前奏望着天空 我想起花瓣试着掉落"
    {RE_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE},
    {DO_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {LA_4, EIGHTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {LA_4, SIXTEENTH_NOTE},
    {SI_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE},
    {LA_4, SIXTEENTH_NOTE}, {SI_4, SIXTEENTH_NOTE}, {LA_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {REST, SIXTEENTH_NOTE}, {FAS_4, SIXTEENTH_NOTE},
    
    // ===== 副歌部分 =====
    // "为你翘课的那一天 花落的那一天"
    {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE},
    {FAS_4, SIXTEENTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {SOL_4, SIXTEENTH_NOTE + SIXTEENTH_NOTE},
    {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE},
    {FAS_4, SIXTEENTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {SOL_4, SIXTEENTH_NOTE + SIXTEENTH_NOTE},
    
    // "教室的那一间 我怎么看不见"
    {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE},
    {FAS_4, SIXTEENTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {SOL_4, SIXTEENTH_NOTE + SIXTEENTH_NOTE},
    {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE},
    {RE_5, SIXTEENTH_NOTE}, {RE_5, EIGHTH_NOTE}, {RE_5, SIXTEENTH_NOTE},
    
    // "消失的下雨天 我好想再淋一遍"
    {REST, SIXTEENTH_NOTE}, {RE_5, SIXTEENTH_NOTE}, {RE_5, SIXTEENTH_NOTE}, {RE_5, SIXTEENTH_NOTE},
    {RE_5, SIXTEENTH_NOTE}, {RE_5, EIGHTH_NOTE}, {RE_5, SIXTEENTH_NOTE + SIXTEENTH_NOTE},
    {RE_5, SIXTEENTH_NOTE}, {RE_5, SIXTEENTH_NOTE}, {RE_5, SIXTEENTH_NOTE},
    {RE_5, SIXTEENTH_NOTE}, {DO_5, SIXTEENTH_NOTE}, {SI_4, SIXTEENTH_NOTE}, {SI_4, SIXTEENTH_NOTE + QUARTER_NOTE + QUARTER_NOTE},
    
    // ===== 桥段 =====
    // "没想到失去的勇气我还留着"
    {REST, QUARTER_NOTE},
    {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE},
    {MI_4, EIGHTH_NOTE}, {FAS_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE},
    {DO_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE + QUARTER_NOTE},

    // "好想再问一遍 你会等待还是离开"
    {REST, QUARTER_NOTE}, 
    {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE}, {SOL_4, SIXTEENTH_NOTE},
    {SI_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE},
    {MI_4, EIGHTH_NOTE}, {FAS_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE},
    {DO_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {LA_4, EIGHTH_NOTE + QUARTER_NOTE + QUARTER_NOTE},
    {REST, QUARTER_NOTE}, {REST, QUARTER_NOTE},

    // "刮风这天 我试过握着你手"
    {SI_4, EIGHTH_NOTE}, {LA_4, EIGHTH_NOTE}, {DO_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE + EIGHTH_NOTE},
    {SOL_4, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {FAS_5, EIGHTH_NOTE}, {SOL_5, EIGHTH_NOTE}, {FAS_5, EIGHTH_NOTE},
    {RE_5, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE + EIGHTH_NOTE},

    // "但偏偏 雨渐渐 大到我看你不见"
    {SOL_4, EIGHTH_NOTE}, {MI_5, EIGHTH_NOTE}, {MI_5, EIGHTH_NOTE + EIGHTH_NOTE},
    {MI_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE + EIGHTH_NOTE},
    {RE_5, EIGHTH_NOTE}, {DO_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {LA_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE},
    {DO_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE + QUARTER_NOTE + QUARTER_NOTE},

    // ===== 结尾部分 =====
    // "还有多久 我才能在你身边"
    {SI_4, EIGHTH_NOTE}, {DOS_5, EIGHTH_NOTE}, {RES_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE + EIGHTH_NOTE},
    {DOS_5, EIGHTH_NOTE}, {RES_5, EIGHTH_NOTE}, {FAS_5, EIGHTH_NOTE}, {LA_5, EIGHTH_NOTE}, {FAS_5, EIGHTH_NOTE},
    {SOL_5, EIGHTH_NOTE}, {SOL_5, EIGHTH_NOTE + QUARTER_NOTE},
    
    // "等到放晴的那天也许我会比较好一点"
    {REST, EIGHTH_NOTE}, {SOL_5, EIGHTH_NOTE}, {SOL_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE},
    {RE_5, EIGHTH_NOTE}, {MI_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {DO_5, EIGHTH_NOTE},
    {LA_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {DO_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE},
    {MI_5, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE}, {MI_5, EIGHTH_NOTE + SIXTEENTH_NOTE}, {FAS_5, SIXTEENTH_NOTE},
    {FAS_5, QUARTER_NOTE},

    // "从前从前 有个人爱你很久 但偏偏 风渐渐 把距离吹的好远"
    {SI_4, EIGHTH_NOTE}, {LA_4, EIGHTH_NOTE}, {DO_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE + EIGHTH_NOTE},
    {SOL_4, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {FAS_5, EIGHTH_NOTE},
    {SOL_5, EIGHTH_NOTE}, {FAS_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE + EIGHTH_NOTE},
    {SOL_4, EIGHTH_NOTE}, {MI_5, EIGHTH_NOTE}, {MI_5, EIGHTH_NOTE + EIGHTH_NOTE}, {MI_5, EIGHTH_NOTE},
    {RE_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE + EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {DO_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE},
    {LA_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {DO_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE + QUARTER_NOTE + QUARTER_NOTE},
    
    // "好不容易 又能在多爱一天 但故事的最后你好像 还是说了拜拜"
    {SI_4, EIGHTH_NOTE}, {DOS_5, EIGHTH_NOTE}, {RES_5, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE + EIGHTH_NOTE},
    {DOS_5, EIGHTH_NOTE}, {RES_5, EIGHTH_NOTE}, {FAS_5, EIGHTH_NOTE}, {LA_5, EIGHTH_NOTE}, {FAS_5, EIGHTH_NOTE},
    {SOL_5, EIGHTH_NOTE}, {SOL_5, EIGHTH_NOTE + QUARTER_NOTE},
    {REST, EIGHTH_NOTE}, {SOL_5, EIGHTH_NOTE}, {SOL_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {RE_5, EIGHTH_NOTE}, {MI_5, EIGHTH_NOTE},
    {RE_5, EIGHTH_NOTE}, {DO_5, EIGHTH_NOTE}, {MI_4, EIGHTH_NOTE}, {FAS_4, EIGHTH_NOTE},
    {SOL_4, EIGHTH_NOTE}, {LA_4, EIGHTH_NOTE}, {SI_4, EIGHTH_NOTE}, {LA_4, EIGHTH_NOTE + QUARTER_NOTE},
    {SI_4, EIGHTH_NOTE}, {SOL_4, EIGHTH_NOTE + QUARTER_NOTE},
};