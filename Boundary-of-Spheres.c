/*
  Boundary of Spheres (구의 경계) — minimal simulator
  ---------------------------------------------------
  - "Sphere" = a frame / worldview / attractor
  - Boundary events happen when pressure, contradiction, or overload crosses thresholds.
  - Goal: keep operation stable by switching resolution, venting excess, or re-centering.

  Build:
    gcc -O2 -std=c11 sphere_boundary.c -o sphere_boundary
  Run:
    ./sphere_boundary

// x0 = pressure, x1 = contradiction, x2 = threshold_dissolve
// return w0 = 1 if (pressure + contradiction) >= threshold_dissolve else 0
check_dissolve:
    add x3, x0, x1
    cmp x3, x2
    cset w0, ge
    ret




*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef enum {
    SPHERE_STABLE = 0,   // 정상: 프레임이 잘 작동
    SPHERE_FLOAT  = 1,   // 부유: 너무 많은 가능성이 열려 경계가 흐릿
    SPHERE_FREEZE = 2,   // 고착: 한 축으로 과도하게 고정
    SPHERE_DISSOLVE = 3  // 해체: 구조가 분해되며 재구성 대기
} SphereState;

typedef enum {
    EVENT_INFO = 0,          // 정보 입력
    EVENT_CONTRADICTION = 1, // 모순 입력
    EVENT_MEANING_OVER = 2,  // 의미 과잉
    EVENT_THREAT = 3,        // 위협(공포/위기 등)
    EVENT_REST = 4           // 휴식/정리(리셋/산책 같은 것)
} EventKind;

typedef struct {
    EventKind kind;
    int32_t intensity;   // 0..1000 정도 가정
} Event;

typedef struct {
    const char *name;

    // Core signals
    int32_t pressure;        // 에너지 압력(과잉, 긴장)
    int32_t contradiction;   // 모순 누적
    int32_t meaning;         // 의미 부여 강도(과잉이면 위험)
    int32_t resolution;      // 해상도(높을수록 정밀하지만 비용↑)
    int32_t fatigue;         // 피로(오버로드)

    // State
    SphereState state;

    // Policy knobs (tunable)
    int32_t th_float;        // 부유 임계
    int32_t th_freeze;       // 고착 임계
    int32_t th_dissolve;     // 해체 임계
} Sphere;

// Clamp helper
static int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static const char* state_name(SphereState s) {
    switch (s) {
        case SPHERE_STABLE: return "STABLE";
        case SPHERE_FLOAT: return "FLOAT";
        case SPHERE_FREEZE: return "FREEZE";
        case SPHERE_DISSOLVE: return "DISSOLVE";
        default: return "UNKNOWN";
    }
}

static const char* event_name(EventKind k) {
    switch (k) {
        case EVENT_INFO: return "INFO";
        case EVENT_CONTRADICTION: return "CONTRADICTION";
        case EVENT_MEANING_OVER: return "MEANING_OVER";
        case EVENT_THREAT: return "THREAT";
        case EVENT_REST: return "REST";
        default: return "UNKNOWN";
    }
}

/*
  "Ignorance Utility" (무지의 활용):
  - Resolution downshift: lower precision to reduce cognitive/compute cost
  - Venting: discard (burn) meaning/contradiction if too heavy
  - Re-centering: reduce pressure via rest
*/
static void apply_ignorance_utility(Sphere *sp, int32_t strength) {
    // strength: 0..1000
    int32_t down = strength / 80;  // small downshift
    if (down > 0) sp->resolution -= down;

    // Vent excess when overloaded
    if (sp->pressure > sp->th_dissolve) {
        sp->meaning -= strength / 8;
        sp->contradiction -= strength / 10;
    }

    // Re-center
    sp->pressure -= strength / 6;
    sp->fatigue  -= strength / 5;

    sp->resolution = clamp_i32(sp->resolution, 1, 100);
    sp->meaning = clamp_i32(sp->meaning, 0, 5000);
    sp->contradiction = clamp_i32(sp->contradiction, 0, 5000);
    sp->pressure = clamp_i32(sp->pressure, 0, 5000);
    sp->fatigue = clamp_i32(sp->fatigue, 0, 5000);
}

/*
  Boundary detection:
  - FLOAT: too many open possibilities (pressure+meaning high, but contradiction moderate)
  - FREEZE: single-axis fixation (meaning high + threat high, resolution stays high, low venting)
  - DISSOLVE: contradiction/pressure beyond thresholds -> break down and reconstruct
*/
static SphereState detect_state(const Sphere *sp) {
    int32_t overload = sp->pressure + sp->fatigue;
    int32_t fixation = sp->meaning + (sp->resolution * 10);
    int32_t chaos = sp->contradiction + sp->pressure;

    if (chaos >= sp->th_dissolve) return SPHERE_DISSOLVE;
    if (fixation >= sp->th_freeze && sp->pressure >= sp->th_freeze / 2) return SPHERE_FREEZE;
    if (overload >= sp->th_float && sp->meaning >= sp->th_float / 3) return SPHERE_FLOAT;
    return SPHERE_STABLE;
}

static void ingest_event(Sphere *sp, Event e) {
    // Basic dynamics: each event shifts the internal signals
    switch (e.kind) {
        case EVENT_INFO:
            // Info increases resolution pressure slightly (more to integrate)
            sp->pressure += e.intensity / 10;
            sp->fatigue  += e.intensity / 15;
            sp->resolution += (e.intensity > 300) ? 1 : 0;
            break;

        case EVENT_CONTRADICTION:
            sp->contradiction += e.intensity / 3;
            sp->pressure += e.intensity / 6;
            sp->fatigue += e.intensity / 8;
            break;

        case EVENT_MEANING_OVER:
            sp->meaning += e.intensity / 2;
            sp->pressure += e.intensity / 5;
            sp->fatigue += e.intensity / 7;
            break;

        case EVENT_THREAT:
            sp->pressure += e.intensity / 2;
            sp->meaning += e.intensity / 6; // threat tends to force meaning-making
            sp->fatigue += e.intensity / 3;
            break;

        case EVENT_REST:
            // Rest: direct vent/recenter
            apply_ignorance_utility(sp, e.intensity);
            break;
    }

    // clamp
    sp->pressure = clamp_i32(sp->pressure, 0, 5000);
    sp->contradiction = clamp_i32(sp->contradiction, 0, 5000);
    sp->meaning = clamp_i32(sp->meaning, 0, 5000);
    sp->resolution = clamp_i32(sp->resolution, 1, 100);
    sp->fatigue = clamp_i32(sp->fatigue, 0, 5000);

    // boundary update
    SphereState new_state = detect_state(sp);

    // transition policy: if you hit risky zones, you can stabilize by "ignorance utility"
    if (new_state == SPHERE_FLOAT) {
        // float => reduce resolution a bit, vent meaning slightly
        apply_ignorance_utility(sp, 240);
    } else if (new_state == SPHERE_FREEZE) {
        // freeze => vent meaning + raise tolerance by downshifting resolution
        apply_ignorance_utility(sp, 420);
    } else if (new_state == SPHERE_DISSOLVE) {
        // dissolve => strong vent, then rebuild at low resolution
        apply_ignorance_utility(sp, 800);
        sp->resolution = 8;        // rebuild at low precision
        sp->meaning = sp->meaning / 2;
        sp->contradiction = sp->contradiction / 2;
        sp->pressure = sp->pressure / 2;
        sp->fatigue = sp->fatigue / 2;
    }

    sp->state = detect_state(sp);
}

static void print_sphere(const Sphere *sp) {
    printf("Sphere[%s] state=%-8s | pressure=%4d contr=%4d meaning=%4d res=%3d fatigue=%4d\n",
           sp->name, state_name(sp->state),
           sp->pressure, sp->contradiction, sp->meaning, sp->resolution, sp->fatigue);
}

int main(void) {
    Sphere s = {
        .name = "GU_BOUNDARY_CORE",
        .pressure = 120,
        .contradiction = 40,
        .meaning = 80,
        .resolution = 18,
        .fatigue = 50,
        .state = SPHERE_STABLE,
        .th_float = 900,
        .th_freeze = 1400,
        .th_dissolve = 2100
    };

    Event script[] = {
        {EVENT_INFO, 300},
        {EVENT_MEANING_OVER, 600},
        {EVENT_CONTRADICTION, 700},
        {EVENT_THREAT, 500},
        {EVENT_INFO, 400},
        {EVENT_CONTRADICTION, 900},
        {EVENT_REST, 600},     // deliberate venting / reset
        {EVENT_MEANING_OVER, 800},
        {EVENT_INFO, 200},
        {EVENT_REST, 300},
    };

    printf("== Boundary of Spheres simulation ==\n");
    print_sphere(&s);

    for (size_t i = 0; i < sizeof(script)/sizeof(script[0]); i++) {
        printf("\n-- ingest: %-14s intensity=%d\n", event_name(script[i].kind), script[i].intensity);
        ingest_event(&s, script[i]);
        print_sphere(&s);
    }

    printf("\nDone.\n");
    return 0;
}