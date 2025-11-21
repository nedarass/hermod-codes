/*
 * Core/Src/shared_data.c
 *
 * Açıklama: Bu dosya sadece global sistem durumu değişkenini (g_system_state)
 * hafızada oluşturur ve başlangıç değerlerini (0) atar.
 */

#include "shared_data.h"
#include <string.h> // memset fonksiyonu için

// ---------------------------------------------------------------------------
// --- GLOBAL DEĞİŞKEN TANIMI ---
// ---------------------------------------------------------------------------
// Hafızada yer ayrılır ve varsayılan olarak sıfırlanır.
// Diğer tüm dosyalar (control.c, sensors.c vb.) bu değişkene 'extern' ile ulaşır.
System_State_t g_system_state = {0};


// ---------------------------------------------------------------------------
// --- BAŞLANGIÇ AYARLARI (Constructor) ---
// ---------------------------------------------------------------------------
// Bu özellik (attribute), main() fonksiyonu başlamadan önce bu kodun çalışmasını sağlar.
// Böylece sistem açıldığında verilerin temiz olduğundan %100 emin oluruz.
void __attribute__((constructor)) init_system_state(void) {
    
    // Tüm yapıyı sıfırla (Güvenlik için)
    memset(&g_system_state, 0, sizeof(System_State_t));
    
    // İsterseniz varsayılan güvenli değerleri burada atayabilirsiniz:
    g_system_state.power_line_status = 1; // Güç var varsayalım
    g_system_state.brake_status = 0;      // Frenler açık (serbest) başlasın
}
