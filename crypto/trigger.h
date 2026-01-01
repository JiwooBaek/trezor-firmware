#pragma once
/*
 * trigger.h — header-only trigger utilities for STM32(libopencm3) + host stub
 *
 * - 공통 매크로만 바꿔서 포트/핀 설정 가능
 * - trigger_init_once() 한 번만 부르면 됨(중복 호출 안전)
 * - trigger_start()/trigger_end() : 핀 High/Low
 * - sleep_us()/sleep_ms() : 바쁜 대기(캘리브레이션 가능)
 * - trig_pulse_us()/trig_pulse_ms() : 펄스 한 번
 * - trigger_frame_mark_begin()/end() : 전체 프레임용 “긴 펄스”
 *
 * 빌드 플래그로 조절:
 *   -DTRIG_GPIO_PORT=GPIOD -DTRIG_GPIO_PIN=GPIO2 -DTRIG_GPIO_RCC=RCC_GPIOD
 *   -DTRIG_FRAME_MARK_MS=5
 *   -DTRIG_US_LOOP_PER_US=30   // CPU 클록에 맞게 조정(기본 러프값)
 *   -DTRIGGER_DISABLE          // 전부 빈 함수(호스트 로그만 남기고 싶을 때)
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ===== 사용자 설정(필요 시 -D로 오버라이드) ===== */
#ifndef TRIG_GPIO_PORT
#define TRIG_GPIO_PORT GPIOD
#endif
#ifndef TRIG_GPIO_PIN
#define TRIG_GPIO_PIN  GPIO2
#endif
#ifndef TRIG_GPIO_RCC
#define TRIG_GPIO_RCC  RCC_GPIOD
#endif

/* 프레임 마크(전체 시작/끝) 기본 길이(ms) */
#ifndef TRIG_FRAME_MARK_MS
#define TRIG_FRAME_MARK_MS 5U
#endif

/* 바쁜 대기 루프 보정치(µs당 반복 수). MCU 클록에 맞게 조정 가능 */
#ifndef TRIG_US_LOOP_PER_US
#define TRIG_US_LOOP_PER_US 30U
#endif
/* ============================================== */

#include <stdint.h>
#include <stdbool.h>

/* ===== 비활성화 모드(스텁) ===== */
#ifdef TRIGGER_DISABLE
static inline void trigger_init_once(void) {}
static inline void trigger_start(void) {}
static inline void trigger_end(void) {}
static inline void sleep_us(uint32_t us) { (void)us; }
static inline void sleep_ms(uint32_t ms) { (void)ms; }
static inline void trig_pulse_us(uint32_t us) { (void)us; }
static inline void trig_pulse_ms(uint32_t ms) { (void)ms; }
static inline void trigger_frame_mark_begin(void) {}
static inline void trigger_frame_mark_end(void) {}
#else

/* ===== STM32(libopencm3) 실기 빌드 ===== */
#if defined(__arm__) || defined(__ARMEL__)
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

static inline void sleep_us(uint32_t us) {
  for (volatile uint32_t i = 0; i < (us * (uint32_t)TRIG_US_LOOP_PER_US); ++i) {
    __asm__ volatile("nop");
  }
}
static inline void sleep_ms(uint32_t ms) {
  while (ms--) sleep_us(1000U);
}

static inline void trigger_init_once(void) {
  static bool inited = false;
  if (inited) return;

  rcc_periph_clock_enable(TRIG_GPIO_RCC);

  /* libopencm3 신형/구형 API 모두 대응 */
#if defined(GPIO_MODE_OUTPUT) && defined(GPIO_PUPD_NONE)
  /* F2/F3/F4 계열 */
  gpio_mode_setup(TRIG_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TRIG_GPIO_PIN);
#else
  /* F1 스타일 구형 매크로 */
  gpio_set_mode(TRIG_GPIO_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, TRIG_GPIO_PIN);
#endif

  /* 기본 Low */
  gpio_clear(TRIG_GPIO_PORT, TRIG_GPIO_PIN);
  inited = true;
}

static inline void trigger_start(void) { gpio_set(TRIG_GPIO_PORT, TRIG_GPIO_PIN); }
static inline void trigger_end(void)   { gpio_clear(TRIG_GPIO_PORT, TRIG_GPIO_PIN); }

/* ===== 호스트(시뮬/유닛 테스트) 빌드 ===== */
#else
static inline void sleep_us(uint32_t us) { (void)us; }
static inline void sleep_ms(uint32_t ms) { (void)ms; }

static inline void trigger_init_once(void) {}
static inline void trigger_start(void) {}
static inline void trigger_end(void) {}
#endif /* platform select */

/* ===== 공통 유틸 ===== */
static inline void trig_pulse_us(uint32_t us) {
  trigger_start();
  sleep_us(us);
  trigger_end();
}

static inline void trig_pulse_ms(uint32_t ms) {
  trigger_start();
  sleep_ms(ms);
  trigger_end();
}

/* 전체 파형의 “프레임 표식” (긴 펄스) */
static inline void trigger_frame_mark_begin(void) { trig_pulse_ms(TRIG_FRAME_MARK_MS); }
static inline void trigger_frame_mark_end(void)   { trig_pulse_ms(TRIG_FRAME_MARK_MS); }

#endif /* TRIGGER_DISABLE */

#ifdef __cplusplus
}
#endif
