//
// Created by sams on 3/4/26.
//

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <stddef.h>
#include <string.h>

#define CONFIG_DIR "/.local/share/SubOptimal/"

// Custom array terminator (distinct from NULL)
#define END_OF_ARRAY "__END_OF_ARRAY__"

// Check if string is the array terminator
static inline int is_end_of_array(const char *s) {
    return s != NULL && strcmp(s, END_OF_ARRAY) == 0;
}

// Safe strncmp that handles NULL and END_OF_ARRAY
// Returns: 1 if match, 0 if no match, -1 if end of array
static inline int strncmp_safe(const char *s1, const char *s2, size_t n) {
    if (s1 == NULL) return 0;  // NULL doesn't match anything
    if (is_end_of_array(s1)) return -1;  // End of array
    return strncmp(s1, s2, n) == 0 ? 1 : 0;
}

// TRANSCRIBE ////////////////////////////////////////
#define HF_BASE_URL "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/"
#define HF_RAW_URL  "https://huggingface.co/ggerganov/whisper.cpp/raw/main/"

static const char* TRANSCRIBE_MODEL_NAMES[] = {
    "tiny",
    "tiny.en",
    "base",
    "base.en",
    "small",
    "small.en",
    "medium",
    "medium.en",
    "large",
    "turbo",
    END_OF_ARRAY
};

static const char* TRANSCRIBE_MODEL_NAMES_FULL[] = {
    "ggml-tiny.bin",
    "ggml-tiny.en.bin",
    "ggml-base.bin",
    "ggml-base.en.bin",
    "ggml-small.bin",
    "ggml-small.en.bin",
    "ggml-medium.bin",
    "ggml-medium.en.bin",
    "ggml-large-v3.bin",
    "ggml-large-v3-turbo.bin",
    END_OF_ARRAY
};

// Whisper language codes aligned with VALID_LANGUAGES indices
// NULL = no Whisper equivalent for this NLLB language
static const char *WHISPER_LANGUAGE_CODES[] = {
    NULL,       // ace_Arab
    NULL,       // ace_Latn
    NULL,       // acm_Arab
    NULL,       // acq_Arab
    NULL,       // aeb_Arab
    "af",       // afr_Latn
    NULL,       // ajp_Arab
    NULL,       // aka_Latn
    "am",       // amh_Ethi
    NULL,       // apc_Arab
    "ar",       // arb_Arab
    NULL,       // arb_Latn
    NULL,       // ars_Arab
    NULL,       // ary_Arab
    NULL,       // arz_Arab
    NULL,       // asm_Beng
    NULL,       // ast_Latn
    NULL,       // awa_Deva
    NULL,       // ayr_Latn
    NULL,       // azb_Arab
    "az",       // azj_Latn
    NULL,       // bak_Cyrl
    NULL,       // bam_Latn
    NULL,       // ban_Latn
    "be",       // bel_Cyrl
    NULL,       // bem_Latn
    "bn",       // ben_Beng
    NULL,       // bho_Deva
    NULL,       // bjn_Arab
    NULL,       // bjn_Latn
    NULL,       // bod_Tibt
    "bs",       // bos_Latn
    NULL,       // bug_Latn
    "bg",       // bul_Cyrl
    "ca",       // cat_Latn
    NULL,       // ceb_Latn
    "cs",       // ces_Latn
    NULL,       // cjk_Latn
    NULL,       // ckb_Arab
    NULL,       // crh_Latn
    "cy",       // cym_Latn
    "da",       // dan_Latn
    "de",       // deu_Latn
    NULL,       // dik_Latn
    NULL,       // dyu_Latn
    NULL,       // dzo_Tibt
    "el",       // ell_Grek
    "en",       // eng_Latn
    NULL,       // epo_Latn
    "et",       // est_Latn
    "eu",       // eus_Latn
    NULL,       // ewe_Latn
    "fo",       // fao_Latn
    NULL,       // fij_Latn
    "fi",       // fin_Latn
    NULL,       // fon_Latn
    "fr",       // fra_Latn
    NULL,       // fur_Latn
    NULL,       // fuv_Latn
    NULL,       // gla_Latn
    NULL,       // gle_Latn
    "gl",       // glg_Latn
    NULL,       // grn_Latn
    "gu",       // guj_Gujr
    "ht",       // hat_Latn
    "ha",       // hau_Latn
    "he",       // heb_Hebr
    "hi",       // hin_Deva
    NULL,       // hne_Deva
    "hr",       // hrv_Latn
    "hu",       // hun_Latn
    "hy",       // hye_Armn
    NULL,       // ibo_Latn
    NULL,       // ilo_Latn
    "id",       // ind_Latn
    "is",       // isl_Latn
    "it",       // ita_Latn
    NULL,       // jav_Latn
    "ja",       // jpn_Jpan
    NULL,       // kab_Latn
    NULL,       // kac_Latn
    NULL,       // kam_Latn
    "kn",       // kan_Knda
    NULL,       // kas_Arab
    NULL,       // kas_Deva
    "ka",       // kat_Geor
    "kk",       // kaz_Cyrl
    NULL,       // kbp_Latn
    NULL,       // kea_Latn
    "km",       // khm_Khmr
    NULL,       // kik_Latn
    NULL,       // kin_Latn
    "ky",       // kir_Cyrl
    NULL,       // kmb_Latn
    NULL,       // kmr_Latn
    NULL,       // knc_Arab
    NULL,       // knc_Latn
    NULL,       // kon_Latn
    "ko",       // kor_Hang
    "lo",       // lao_Laoo
    NULL,       // lij_Latn
    NULL,       // lim_Latn
    NULL,       // lin_Latn
    "lt",       // lit_Latn
    NULL,       // lmo_Latn
    NULL,       // ltg_Latn
    NULL,       // ltz_Latn
    NULL,       // lua_Latn
    NULL,       // lug_Latn
    NULL,       // luo_Latn
    NULL,       // lus_Latn
    NULL,       // lvs_Latn
    NULL,       // mag_Deva
    NULL,       // mai_Deva
    "ml",       // mal_Mlym
    "mr",       // mar_Deva
    NULL,       // min_Arab
    NULL,       // min_Latn
    "mk",       // mkd_Cyrl
    "mt",       // mlt_Latn
    NULL,       // mni_Beng
    NULL,       // mos_Latn
    "mi",       // mri_Latn
    "my",       // mya_Mymr
    "nl",       // nld_Latn
    NULL,       // nno_Latn
    "no",       // nob_Latn
    NULL,       // npi_Deva
    NULL,       // nso_Latn
    NULL,       // nus_Latn
    NULL,       // nya_Latn
    NULL,       // oci_Latn
    NULL,       // ory_Orya
    NULL,       // pag_Latn
    "pa",       // pan_Guru
    NULL,       // pap_Latn
    "fa",       // pes_Arab
    NULL,       // plt_Latn
    "pl",       // pol_Latn
    "pt",       // por_Latn
    NULL,       // prs_Arab
    NULL,       // pbt_Arab
    NULL,       // quy_Latn
    "ro",       // ron_Latn
    NULL,       // run_Latn
    "ru",       // rus_Cyrl
    NULL,       // sag_Latn
    NULL,       // san_Deva
    NULL,       // sat_Olck
    NULL,       // scn_Latn
    NULL,       // shn_Mymr
    "si",       // sin_Sinh
    "sk",       // slk_Latn
    "sl",       // slv_Latn
    NULL,       // smo_Latn
    NULL,       // sna_Latn
    "sd",       // snd_Arab
    "so",       // som_Latn
    NULL,       // sot_Latn
    "es",       // spa_Latn
    NULL,       // srd_Latn
    "sr",       // srp_Cyrl
    NULL,       // ssw_Latn
    NULL,       // sun_Latn
    "sv",       // swe_Latn
    "sw",       // swh_Latn
    NULL,       // szl_Latn
    "ta",       // tam_Taml
    NULL,       // taq_Latn
    NULL,       // taq_Tfng
    "tt",       // tat_Cyrl
    "te",       // tel_Telu
    "tg",       // tgk_Cyrl
    "tl",       // tgl_Latn
    "th",       // tha_Thai
    NULL,       // tir_Ethi
    NULL,       // tpi_Latn
    NULL,       // tsn_Latn
    NULL,       // tso_Latn
    "tk",       // tuk_Latn
    NULL,       // tum_Latn
    "tr",       // tur_Latn
    NULL,       // twi_Latn
    NULL,       // tzm_Tfng
    NULL,       // uig_Arab
    "uk",       // ukr_Cyrl
    NULL,       // umb_Latn
    "ur",       // urd_Arab
    "uz",       // uzn_Latn
    NULL,       // vec_Latn
    "vi",       // vie_Latn
    NULL,       // war_Latn
    NULL,       // wol_Latn
    NULL,       // xho_Latn
    "yi",       // ydd_Hebr
    "yo",       // yor_Latn
    NULL,       // yue_Hant
    "zh",       // zho_Hans
    NULL,       // zho_Hant
    NULL,       // zsm_Latn
    NULL,       // zul_Latn
    END_OF_ARRAY
};

static const char *WHISPER_LANGUAGE_NAMES[] = {
    NULL,       // ace_Arab
    NULL,       // ace_Latn
    NULL,       // acm_Arab
    NULL,       // acq_Arab
    NULL,       // aeb_Arab
    "Afrikaans",
    NULL,       // ajp_Arab
    NULL,       // aka_Latn
    "Amharic",
    NULL,       // apc_Arab
    "Arabic",
    NULL,       // arb_Latn
    NULL,       // ars_Arab
    NULL,       // ary_Arab
    NULL,       // arz_Arab
    NULL,       // asm_Beng
    NULL,       // ast_Latn
    NULL,       // awa_Deva
    NULL,       // ayr_Latn
    NULL,       // azb_Arab
    "Azerbaijani",
    NULL,       // bak_Cyrl
    NULL,       // bam_Latn
    NULL,       // ban_Latn
    "Belarusian",
    NULL,       // bem_Latn
    "Bengali",
    NULL,       // bho_Deva
    NULL,       // bjn_Arab
    NULL,       // bjn_Latn
    NULL,       // bod_Tibt
    "Bosnian",
    NULL,       // bug_Latn
    "Bulgarian",
    "Catalan",
    NULL,       // ceb_Latn
    "Czech",
    NULL,       // cjk_Latn
    NULL,       // ckb_Arab
    NULL,       // crh_Latn
    "Welsh",
    "Danish",
    "German",
    NULL,       // dik_Latn
    NULL,       // dyu_Latn
    NULL,       // dzo_Tibt
    "Greek",
    "English",
    NULL,       // epo_Latn
    "Estonian",
    "Basque",
    NULL,       // ewe_Latn
    "Faroese",
    NULL,       // fij_Latn
    "Finnish",
    NULL,       // fon_Latn
    "French",
    NULL,       // fur_Latn
    NULL,       // fuv_Latn
    NULL,       // gla_Latn
    NULL,       // gle_Latn
    "Galician",
    NULL,       // grn_Latn
    "Gujarati",
    "Haitian Creole",
    "Hausa",
    "Hebrew",
    "Hindi",
    NULL,       // hne_Deva
    "Croatian",
    "Hungarian",
    "Armenian",
    NULL,       // ibo_Latn
    NULL,       // ilo_Latn
    "Indonesian",
    "Icelandic",
    "Italian",
    NULL,       // jav_Latn
    "Japanese",
    NULL,       // kab_Latn
    NULL,       // kac_Latn
    NULL,       // kam_Latn
    "Kannada",
    NULL,       // kas_Arab
    NULL,       // kas_Deva
    "Georgian",
    "Kazakh",
    NULL,       // kbp_Latn
    NULL,       // kea_Latn
    "Khmer",
    NULL,       // kik_Latn
    NULL,       // kin_Latn
    "Kyrgyz",
    NULL,       // kmb_Latn
    NULL,       // kmr_Latn
    NULL,       // knc_Arab
    NULL,       // knc_Latn
    NULL,       // kon_Latn
    "Korean",
    "Lao",
    NULL,       // lij_Latn
    NULL,       // lim_Latn
    NULL,       // lin_Latn
    "Lithuanian",
    NULL,       // lmo_Latn
    NULL,       // ltg_Latn
    NULL,       // ltz_Latn
    NULL,       // lua_Latn
    NULL,       // lug_Latn
    NULL,       // luo_Latn
    NULL,       // lus_Latn
    NULL,       // lvs_Latn
    NULL,       // mag_Deva
    NULL,       // mai_Deva
    "Malayalam",
    "Marathi",
    NULL,       // min_Arab
    NULL,       // min_Latn
    "Macedonian",
    "Maltese",
    NULL,       // mni_Beng
    NULL,       // mos_Latn
    "Maori",
    "Myanmar",
    "Dutch",
    NULL,       // nno_Latn
    "Norwegian",
    NULL,       // npi_Deva
    NULL,       // nso_Latn
    NULL,       // nus_Latn
    NULL,       // nya_Latn
    NULL,       // oci_Latn
    NULL,       // ory_Orya
    NULL,       // pag_Latn
    "Punjabi",
    NULL,       // pap_Latn
    "Persian",
    NULL,       // plt_Latn
    "Polish",
    "Portuguese",
    NULL,       // prs_Arab
    NULL,       // pbt_Arab
    NULL,       // quy_Latn
    "Romanian",
    NULL,       // run_Latn
    "Russian",
    NULL,       // sag_Latn
    NULL,       // san_Deva
    NULL,       // sat_Olck
    NULL,       // scn_Latn
    NULL,       // shn_Mymr
    "Sinhala",
    "Slovak",
    "Slovenian",
    NULL,       // smo_Latn
    NULL,       // sna_Latn
    "Sindhi",
    "Somali",
    NULL,       // sot_Latn
    "Spanish",
    NULL,       // srd_Latn
    "Serbian",
    NULL,       // ssw_Latn
    NULL,       // sun_Latn
    "Swedish",
    "Swahili",
    NULL,       // szl_Latn
    "Tamil",
    NULL,       // taq_Latn
    NULL,       // taq_Tfng
    "Tatar",
    "Telugu",
    "Tajik",
    "Tagalog",
    "Thai",
    NULL,       // tir_Ethi
    NULL,       // tpi_Latn
    NULL,       // tsn_Latn
    NULL,       // tso_Latn
    "Turkmen",
    NULL,       // tum_Latn
    "Turkish",
    NULL,       // twi_Latn
    NULL,       // tzm_Tfng
    NULL,       // uig_Arab
    "Ukrainian",
    NULL,       // umb_Latn
    "Urdu",
    "Uzbek",
    NULL,       // vec_Latn
    "Vietnamese",
    NULL,       // war_Latn
    NULL,       // wol_Latn
    NULL,       // xho_Latn
    "Yiddish",
    "Yoruba",
    NULL,       // yue_Hant
    "Chinese",
    NULL,       // zho_Hant
    NULL,       // zsm_Latn
    NULL,       // zul_Latn
    END_OF_ARRAY
};

#define VAD_MODEL_NAME "ggml-silero-v6.2.0.bin"
#define VAD_MODEL_URL  "https://huggingface.co/ggml-org/whisper-vad/resolve/main/ggml-silero-v6.2.0.bin"


// TRANSLATE ////////////////////////////////////////
// NLLB model (fallback for non-English pairs)
#define NLLB_MODEL_DIR CONFIG_DIR "nllb-200-distilled-600M-ct2/"
#define NLLB_MODEL_NAME  "model.bin"
#define NLLB_MODEL_URL   "https://huggingface.co/JustFrederik/nllb-200-distilled-600M-ct2/resolve/main/model.bin"
#define NLLB_SPM_NAME    "sentencepiece.bpe.model"
#define NLLB_SPM_URL     "https://huggingface.co/JustFrederik/nllb-200-distilled-600M-ct2/resolve/main/sentencepiece.bpe.model"
#define NLLB_CONFIG_NAME "config.json"
#define NLLB_CONFIG_URL  "https://huggingface.co/JustFrederik/nllb-200-distilled-600M-ct2/resolve/main/config.json"


// NLLB TRANSLATION LANGUAGES ////////////////////////////////////////
static const char *VALID_LANGUAGES[] = {
    "ace_Arab",
    "ace_Latn",
    "acm_Arab",
    "acq_Arab",
    "aeb_Arab",
    "afr_Latn",
    "ajp_Arab",
    "aka_Latn",
    "amh_Ethi",
    "apc_Arab",
    "arb_Arab",
    "arb_Latn",
    "ars_Arab",
    "ary_Arab",
    "arz_Arab",
    "asm_Beng",
    "ast_Latn",
    "awa_Deva",
    "ayr_Latn",
    "azb_Arab",
    "azj_Latn",
    "bak_Cyrl",
    "bam_Latn",
    "ban_Latn",
    "bel_Cyrl",
    "bem_Latn",
    "ben_Beng",
    "bho_Deva",
    "bjn_Arab",
    "bjn_Latn",
    "bod_Tibt",
    "bos_Latn",
    "bug_Latn",
    "bul_Cyrl",
    "cat_Latn",
    "ceb_Latn",
    "ces_Latn",
    "cjk_Latn",
    "ckb_Arab",
    "crh_Latn",
    "cym_Latn",
    "dan_Latn",
    "deu_Latn",
    "dik_Latn",
    "dyu_Latn",
    "dzo_Tibt",
    "ell_Grek",
    "eng_Latn",
    "epo_Latn",
    "est_Latn",
    "eus_Latn",
    "ewe_Latn",
    "fao_Latn",
    "fij_Latn",
    "fin_Latn",
    "fon_Latn",
    "fra_Latn",
    "fur_Latn",
    "fuv_Latn",
    "gla_Latn",
    "gle_Latn",
    "glg_Latn",
    "grn_Latn",
    "guj_Gujr",
    "hat_Latn",
    "hau_Latn",
    "heb_Hebr",
    "hin_Deva",
    "hne_Deva",
    "hrv_Latn",
    "hun_Latn",
    "hye_Armn",
    "ibo_Latn",
    "ilo_Latn",
    "ind_Latn",
    "isl_Latn",
    "ita_Latn",
    "jav_Latn",
    "jpn_Jpan",
    "kab_Latn",
    "kac_Latn",
    "kam_Latn",
    "kan_Knda",
    "kas_Arab",
    "kas_Deva",
    "kat_Geor",
    "kaz_Cyrl",
    "kbp_Latn",
    "kea_Latn",
    "khm_Khmr",
    "kik_Latn",
    "kin_Latn",
    "kir_Cyrl",
    "kmb_Latn",
    "kmr_Latn",
    "knc_Arab",
    "knc_Latn",
    "kon_Latn",
    "kor_Hang",
    "lao_Laoo",
    "lij_Latn",
    "lim_Latn",
    "lin_Latn",
    "lit_Latn",
    "lmo_Latn",
    "ltg_Latn",
    "ltz_Latn",
    "lua_Latn",
    "lug_Latn",
    "luo_Latn",
    "lus_Latn",
    "lvs_Latn",
    "mag_Deva",
    "mai_Deva",
    "mal_Mlym",
    "mar_Deva",
    "min_Arab",
    "min_Latn",
    "mkd_Cyrl",
    "mlt_Latn",
    "mni_Beng",
    "mos_Latn",
    "mri_Latn",
    "mya_Mymr",
    "nld_Latn",
    "nno_Latn",
    "nob_Latn",
    "npi_Deva",
    "nso_Latn",
    "nus_Latn",
    "nya_Latn",
    "oci_Latn",
    "ory_Orya",
    "pag_Latn",
    "pan_Guru",
    "pap_Latn",
    "pes_Arab",
    "plt_Latn",
    "pol_Latn",
    "por_Latn",
    "prs_Arab",
    "pbt_Arab",
    "quy_Latn",
    "ron_Latn",
    "run_Latn",
    "rus_Cyrl",
    "sag_Latn",
    "san_Deva",
    "sat_Olck",
    "scn_Latn",
    "shn_Mymr",
    "sin_Sinh",
    "slk_Latn",
    "slv_Latn",
    "smo_Latn",
    "sna_Latn",
    "snd_Arab",
    "som_Latn",
    "sot_Latn",
    "spa_Latn",
    "srd_Latn",
    "srp_Cyrl",
    "ssw_Latn",
    "sun_Latn",
    "swe_Latn",
    "swh_Latn",
    "szl_Latn",
    "tam_Taml",
    "taq_Latn",
    "taq_Tfng",
    "tat_Cyrl",
    "tel_Telu",
    "tgk_Cyrl",
    "tgl_Latn",
    "tha_Thai",
    "tir_Ethi",
    "tpi_Latn",
    "tsn_Latn",
    "tso_Latn",
    "tuk_Latn",
    "tum_Latn",
    "tur_Latn",
    "twi_Latn",
    "tzm_Tfng",
    "uig_Arab",
    "ukr_Cyrl",
    "umb_Latn",
    "urd_Arab",
    "uzn_Latn",
    "vec_Latn",
    "vie_Latn",
    "war_Latn",
    "wol_Latn",
    "xho_Latn",
    "ydd_Hebr",
    "yor_Latn",
    "yue_Hant",
    "zho_Hans",
    "zho_Hant",
    "zsm_Latn",
    "zul_Latn",
    END_OF_ARRAY
};

static const char *LANGUAGE_NAMES[] = {
    "Acehnese (Arabic script)",
    "Acehnese (Latin script)",
    "Mesopotamian Arabic",
    "Ta'izzi-Adeni Arabic",
    "Tunisian Arabic",
    "Afrikaans",
    "South Levantine Arabic",
    "Akan",
    "Amharic",
    "North Levantine Arabic",
    "Modern Standard Arabic",
    "Modern Standard Arabic (Romanized)",
    "Najdi Arabic",
    "Moroccan Arabic",
    "Egyptian Arabic",
    "Assamese",
    "Asturian",
    "Awadhi",
    "Central Aymara",
    "South Azerbaijani",
    "North Azerbaijani",
    "Bashkir",
    "Bambara",
    "Balinese",
    "Belarusian",
    "Bemba",
    "Bengali",
    "Bhojpuri",
    "Banjar (Arabic script)",
    "Banjar (Latin script)",
    "Standard Tibetan",
    "Bosnian",
    "Buginese",
    "Bulgarian",
    "Catalan",
    "Cebuano",
    "Czech",
    "Chokwe",
    "Central Kurdish",
    "Crimean Tatar",
    "Welsh",
    "Danish",
    "German",
    "Southwestern Dinka",
    "Dyula",
    "Dzongkha",
    "Greek",
    "English",
    "Esperanto",
    "Estonian",
    "Basque",
    "Ewe",
    "Faroese",
    "Fijian",
    "Finnish",
    "Fon",
    "French",
    "Friulian",
    "Nigerian Fulfulde",
    "Scottish Gaelic",
    "Irish",
    "Galician",
    "Guarani",
    "Gujarati",
    "Haitian Creole",
    "Hausa",
    "Hebrew",
    "Hindi",
    "Chhattisgarhi",
    "Croatian",
    "Hungarian",
    "Armenian",
    "Igbo",
    "Ilocano",
    "Indonesian",
    "Icelandic",
    "Italian",
    "Javanese",
    "Japanese",
    "Kabyle",
    "Jingpho",
    "Kamba",
    "Kannada",
    "Kashmiri (Arabic script)",
    "Kashmiri (Devanagari script)",
    "Georgian",
    "Kazakh",
    "Kabiyè",
    "Kabuverdianu",
    "Khmer",
    "Kikuyu",
    "Kinyarwanda",
    "Kyrgyz",
    "Kimbundu",
    "Northern Kurdish",
    "Central Kanuri (Arabic script)",
    "Central Kanuri (Latin script)",
    "Kikongo",
    "Korean",
    "Lao",
    "Ligurian",
    "Limburgish",
    "Lingala",
    "Lithuanian",
    "Lombard",
    "Latgalian",
    "Luxembourgish",
    "Luba-Kasai",
    "Ganda",
    "Luo",
    "Mizo",
    "Standard Latvian",
    "Magahi",
    "Maithili",
    "Malayalam",
    "Marathi",
    "Minangkabau (Arabic script)",
    "Minangkabau (Latin script)",
    "Macedonian",
    "Maltese",
    "Meitei (Bengali script)",
    "Mossi",
    "Maori",
    "Burmese",
    "Dutch",
    "Norwegian Nynorsk",
    "Norwegian Bokmål",
    "Nepali",
    "Northern Sotho",
    "Nuer",
    "Nyanja",
    "Occitan",
    "Odia",
    "Pangasinan",
    "Eastern Panjabi",
    "Papiamento",
    "Western Persian",
    "Plateau Malagasy",
    "Polish",
    "Portuguese",
    "Dari",
    "Southern Pashto",
    "Ayacucho Quechua",
    "Romanian",
    "Rundi",
    "Russian",
    "Sango",
    "Sanskrit",
    "Santali",
    "Sicilian",
    "Shan",
    "Sinhala",
    "Slovak",
    "Slovenian",
    "Samoan",
    "Shona",
    "Sindhi",
    "Somali",
    "Southern Sotho",
    "Spanish",
    "Sardinian",
    "Serbian",
    "Swati",
    "Sundanese",
    "Swedish",
    "Swahili",
    "Silesian",
    "Tamil",
    "Tamasheq (Latin script)",
    "Tamasheq (Tifinagh script)",
    "Tatar",
    "Telugu",
    "Tajik",
    "Tagalog",
    "Thai",
    "Tigrinya",
    "Tok Pisin",
    "Tswana",
    "Tsonga",
    "Turkmen",
    "Tumbuka",
    "Turkish",
    "Twi",
    "Central Atlas Tamazight",
    "Uyghur",
    "Ukrainian",
    "Umbundu",
    "Urdu",
    "Northern Uzbek",
    "Venetian",
    "Vietnamese",
    "Waray",
    "Wolof",
    "Xhosa",
    "Eastern Yiddish",
    "Yoruba",
    "Yue Chinese",
    "Chinese (Simplified)",
    "Chinese (Traditional)",
    "Standard Malay",
    "Zulu",
    END_OF_ARRAY
};

// Helsinki en -> lang model URLs (hosted on HuggingFace: Sams200)
// NULL = model not available for this direction/language.
static const char *HELSINKI_EN_TO_LANG_URL[] = {
    NULL,                                                   // ace_Arab
    NULL,                                                   // ace_Latn
    NULL,                                                   // acm_Arab
    NULL,                                                   // acq_Arab
    NULL,                                                   // aeb_Arab
    "https://huggingface.co/Sams200/opus-mt-en-af/resolve/main",
    NULL,                                                   // ajp_Arab
    NULL,                                                   // aka_Latn
    NULL,                                                   // amh_Ethi
    NULL,                                                   // apc_Arab
    "https://huggingface.co/Sams200/opus-mt-en-ar/resolve/main",
    NULL,                                                   // arb_Latn
    NULL,                                                   // ars_Arab
    NULL,                                                   // ary_Arab
    NULL,                                                   // arz_Arab
    NULL,                                                   // asm_Beng
    NULL,                                                   // ast_Latn
    NULL,                                                   // awa_Deva
    NULL,                                                   // ayr_Latn
    NULL,                                                   // azb_Arab
    "https://huggingface.co/Sams200/opus-mt-en-az/resolve/main",
    NULL,                                                   // bak_Cyrl
    NULL,                                                   // bam_Latn
    NULL,                                                   // ban_Latn
    NULL,                                                   // bel_Cyrl
    NULL,                                                   // bem_Latn
    NULL,                                                   // ben_Beng
    NULL,                                                   // bho_Deva
    NULL,                                                   // bjn_Arab
    NULL,                                                   // bjn_Latn
    NULL,                                                   // bod_Tibt
    NULL,                                                   // bos_Latn
    NULL,                                                   // bug_Latn
    "https://huggingface.co/Sams200/opus-mt-en-bg/resolve/main",
    "https://huggingface.co/Sams200/opus-mt-en-ca/resolve/main",
    NULL,                                                   // ceb_Latn
    NULL,                                                   // ces_Latn (not uploaded)
    NULL,                                                   // cjk_Latn
    NULL,                                                   // ckb_Arab
    NULL,                                                   // crh_Latn
    NULL,                                                   // cym_Latn (not uploaded)
    NULL,                                                   // dan_Latn (not uploaded)
    NULL,                                                   // deu_Latn (not uploaded)
    NULL,                                                   // dik_Latn
    NULL,                                                   // dyu_Latn
    NULL,                                                   // dzo_Tibt
    "https://huggingface.co/Sams200/opus-mt-en-el/resolve/main",
    NULL,                                                   // eng_Latn
    NULL,                                                   // epo_Latn
    "https://huggingface.co/Sams200/opus-mt-en-et/resolve/main",
    "https://huggingface.co/Sams200/opus-mt-en-eu/resolve/main",
    NULL,                                                   // ewe_Latn
    NULL,                                                   // fao_Latn (not uploaded)
    NULL,                                                   // fij_Latn
    "https://huggingface.co/Sams200/opus-mt-en-fi/resolve/main",
    NULL,                                                   // fon_Latn
    "https://huggingface.co/Sams200/opus-mt-en-fr/resolve/main",
    NULL,                                                   // fur_Latn
    NULL,                                                   // fuv_Latn
    NULL,                                                   // gla_Latn
    NULL,                                                   // gle_Latn
    "https://huggingface.co/Sams200/opus-mt-en-gl/resolve/main",
    NULL,                                                   // grn_Latn
    NULL,                                                   // guj_Gujr
    NULL,                                                   // hat_Latn (not uploaded)
    NULL,                                                   // hau_Latn (not uploaded)
    "https://huggingface.co/Sams200/opus-mt-en-he/resolve/main",
    "https://huggingface.co/Sams200/opus-mt-en-hi/resolve/main",
    NULL,                                                   // hne_Deva
    NULL,                                                   // hrv_Latn (not uploaded)
    "https://huggingface.co/Sams200/opus-mt-en-hu/resolve/main",
    "https://huggingface.co/Sams200/opus-mt-en-hy/resolve/main",
    NULL,                                                   // ibo_Latn
    NULL,                                                   // ilo_Latn
    "https://huggingface.co/Sams200/opus-mt-en-id/resolve/main",
    "https://huggingface.co/Sams200/opus-mt-en-is/resolve/main",
    "https://huggingface.co/Sams200/opus-mt-en-it/resolve/main",
    NULL,                                                   // jav_Latn
    NULL,                                                   // jpn_Jpan (not uploaded)
    NULL,                                                   // kab_Latn
    NULL,                                                   // kac_Latn
    NULL,                                                   // kam_Latn
    NULL,                                                   // kan_Knda
    NULL,                                                   // kas_Arab
    NULL,                                                   // kas_Deva
    NULL,                                                   // kat_Geor
    NULL,                                                   // kaz_Cyrl
    NULL,                                                   // kbp_Latn
    NULL,                                                   // kea_Latn
    NULL,                                                   // khm_Khmr
    NULL,                                                   // kik_Latn
    NULL,                                                   // kin_Latn
    NULL,                                                   // kir_Cyrl
    NULL,                                                   // kmb_Latn
    NULL,                                                   // kmr_Latn
    NULL,                                                   // knc_Arab
    NULL,                                                   // knc_Latn
    NULL,                                                   // kon_Latn
    NULL,                                                   // kor_Hang (not uploaded)
    NULL,                                                   // lao_Laoo
    NULL,                                                   // lij_Latn
    NULL,                                                   // lim_Latn
    NULL,                                                   // lin_Latn
    NULL,                                                   // lit_Latn (not uploaded)
    NULL,                                                   // lmo_Latn
    NULL,                                                   // ltg_Latn
    NULL,                                                   // ltz_Latn
    NULL,                                                   // lua_Latn
    NULL,                                                   // lug_Latn
    NULL,                                                   // luo_Latn
    NULL,                                                   // lus_Latn
    NULL,                                                   // lvs_Latn (not uploaded)
    NULL,                                                   // mag_Deva
    NULL,                                                   // mai_Deva
    NULL,                                                   // mal_Mlym (not uploaded)
    NULL,                                                   // mar_Deva (not uploaded)
    NULL,                                                   // min_Arab
    NULL,                                                   // min_Latn
    "https://huggingface.co/Sams200/opus-mt-en-mk/resolve/main",
    "https://huggingface.co/Sams200/opus-mt-en-mt/resolve/main",
    NULL,                                                   // mni_Beng
    NULL,                                                   // mos_Latn
    NULL,                                                   // mri_Latn
    NULL,                                                   // mya_Mymr
    "https://huggingface.co/Sams200/opus-mt-en-nl/resolve/main",
    NULL,                                                   // nno_Latn
    NULL,                                                   // nob_Latn (not uploaded)
    NULL,                                                   // npi_Deva
    NULL,                                                   // nso_Latn
    NULL,                                                   // nus_Latn
    NULL,                                                   // nya_Latn
    NULL,                                                   // oci_Latn
    NULL,                                                   // ory_Orya
    NULL,                                                   // pag_Latn
    NULL,                                                   // pan_Guru (not uploaded)
    NULL,                                                   // pap_Latn
    NULL,                                                   // pes_Arab (not uploaded)
    NULL,                                                   // plt_Latn
    NULL,                                                   // pol_Latn (not uploaded)
    NULL,                                                   // por_Latn (not uploaded)
    NULL,                                                   // prs_Arab
    NULL,                                                   // pbt_Arab
    NULL,                                                   // quy_Latn
    "https://huggingface.co/Sams200/opus-mt-en-ro/resolve/main",
    NULL,                                                   // run_Latn
    "https://huggingface.co/Sams200/opus-mt-en-ru/resolve/main",
    NULL,                                                   // sag_Latn
    NULL,                                                   // san_Deva
    NULL,                                                   // sat_Olck
    NULL,                                                   // scn_Latn
    NULL,                                                   // shn_Mymr
    NULL,                                                   // sin_Sinh (not uploaded)
    "https://huggingface.co/Sams200/opus-mt-en-sk/resolve/main",
    NULL,                                                   // slv_Latn (not uploaded)
    NULL,                                                   // smo_Latn
    NULL,                                                   // sna_Latn
    NULL,                                                   // snd_Arab
    NULL,                                                   // som_Latn
    NULL,                                                   // sot_Latn
    "https://huggingface.co/Sams200/opus-mt-en-es/resolve/main",
    NULL,                                                   // srd_Latn
    NULL,                                                   // srp_Cyrl (not uploaded)
    NULL,                                                   // ssw_Latn
    NULL,                                                   // sun_Latn
    "https://huggingface.co/Sams200/opus-mt-en-sv/resolve/main",
    "https://huggingface.co/Sams200/opus-mt-en-sw/resolve/main",
    NULL,                                                   // szl_Latn
    NULL,                                                   // tam_Taml (not uploaded)
    NULL,                                                   // taq_Latn
    NULL,                                                   // taq_Tfng
    NULL,                                                   // tat_Cyrl
    NULL,                                                   // tel_Telu
    NULL,                                                   // tgk_Cyrl
    "https://huggingface.co/Sams200/opus-mt-en-tl/resolve/main",
    NULL,                                                   // tha_Thai (not uploaded)
    NULL,                                                   // tir_Ethi
    NULL,                                                   // tpi_Latn
    NULL,                                                   // tsn_Latn
    NULL,                                                   // tso_Latn
    NULL,                                                   // tuk_Latn
    NULL,                                                   // tum_Latn
    NULL,                                                   // tur_Latn (not uploaded)
    NULL,                                                   // twi_Latn
    NULL,                                                   // tzm_Tfng
    NULL,                                                   // uig_Arab
    "https://huggingface.co/Sams200/opus-mt-en-uk/resolve/main",
    NULL,                                                   // umb_Latn
    "https://huggingface.co/Sams200/opus-mt-en-ur/resolve/main",
    NULL,                                                   // uzn_Latn
    NULL,                                                   // vec_Latn
    "https://huggingface.co/Sams200/opus-mt-en-vi/resolve/main",
    NULL,                                                   // war_Latn
    NULL,                                                   // wol_Latn
    NULL,                                                   // xho_Latn
    NULL,                                                   // ydd_Hebr
    NULL,                                                   // yor_Latn
    NULL,                                                   // yue_Hant
    "https://huggingface.co/Sams200/opus-mt-en-zh/resolve/main",
    NULL,                                                   // zho_Hant
    NULL,                                                   // zsm_Latn (not uploaded)
    NULL,                                                   // zul_Latn
    END_OF_ARRAY
};

// Helsinki lang -> en model URLs, aligned with VALID_LANGUAGES indices.
// NULL = model not uploaded or not available.
static const char *HELSINKI_LANG_TO_EN_URL[] = {
    NULL,                                                               // ace_Arab
    NULL,                                                               // ace_Latn
    NULL,                                                               // acm_Arab
    NULL,                                                               // acq_Arab
    NULL,                                                               // aeb_Arab
    "https://huggingface.co/Sams200/opus-mt-af-en/resolve/main",       // afr_Latn
    NULL,                                                               // ajp_Arab
    NULL,                                                               // aka_Latn
    NULL,                                                               // amh_Ethi
    NULL,                                                               // apc_Arab
    "https://huggingface.co/Sams200/opus-mt-ar-en/resolve/main",       // arb_Arab
    NULL,                                                               // arb_Latn
    NULL,                                                               // ars_Arab
    NULL,                                                               // ary_Arab
    NULL,                                                               // arz_Arab
    NULL,                                                               // asm_Beng
    NULL,                                                               // ast_Latn
    NULL,                                                               // awa_Deva
    NULL,                                                               // ayr_Latn
    NULL,                                                               // azb_Arab
    "https://huggingface.co/Sams200/opus-mt-az-en/resolve/main",       // azj_Latn
    NULL,                                                               // bak_Cyrl
    NULL,                                                               // bam_Latn
    NULL,                                                               // ban_Latn
    NULL,                                                               // bel_Cyrl
    NULL,                                                               // bem_Latn
    NULL,                                                               // ben_Beng
    NULL,                                                               // bho_Deva
    NULL,                                                               // bjn_Arab
    NULL,                                                               // bjn_Latn
    NULL,                                                               // bod_Tibt
    NULL,                                                               // bos_Latn
    NULL,                                                               // bug_Latn
    "https://huggingface.co/Sams200/opus-mt-bg-en/resolve/main",       // bul_Cyrl
    "https://huggingface.co/Sams200/opus-mt-ca-en/resolve/main",       // cat_Latn
    NULL,                                                               // ceb_Latn
    NULL,                                                               // ces_Latn
    NULL,                                                               // cjk_Latn
    NULL,                                                               // ckb_Arab
    NULL,                                                               // crh_Latn
    NULL,                                                               // cym_Latn
    NULL,                                                               // dan_Latn
    NULL,                                                               // deu_Latn
    NULL,                                                               // dik_Latn
    NULL,                                                               // dyu_Latn
    NULL,                                                               // dzo_Tibt
    "https://huggingface.co/Sams200/opus-mt-el-en/resolve/main",       // ell_Grek
    NULL,                                                               // eng_Latn
    NULL,                                                               // epo_Latn
    "https://huggingface.co/Sams200/opus-mt-et-en/resolve/main",       // est_Latn
    "https://huggingface.co/Sams200/opus-mt-eu-en/resolve/main",       // eus_Latn
    NULL,                                                               // ewe_Latn
    NULL,                                                               // fao_Latn
    NULL,                                                               // fij_Latn
    NULL,                                                               // fin_Latn
    NULL,                                                               // fon_Latn
    "https://huggingface.co/Sams200/opus-mt-fr-en/resolve/main",       // fra_Latn
    NULL,                                                               // fur_Latn
    NULL,                                                               // fuv_Latn
    NULL,                                                               // gla_Latn
    NULL,                                                               // gle_Latn
    "https://huggingface.co/Sams200/opus-mt-gl-en/resolve/main",       // glg_Latn
    NULL,                                                               // grn_Latn
    NULL,                                                               // guj_Gujr
    NULL,                                                               // hat_Latn
    NULL,                                                               // hau_Latn
    "https://huggingface.co/Sams200/opus-mt-he-en/resolve/main",       // heb_Hebr
    "https://huggingface.co/Sams200/opus-mt-hi-en/resolve/main",       // hin_Deva
    NULL,                                                               // hne_Deva
    NULL,                                                               // hrv_Latn
    "https://huggingface.co/Sams200/opus-mt-hu-en/resolve/main",       // hun_Latn
    "https://huggingface.co/Sams200/opus-mt-hy-en/resolve/main",       // hye_Armn
    NULL,                                                               // ibo_Latn
    NULL,                                                               // ilo_Latn
    "https://huggingface.co/Sams200/opus-mt-id-en/resolve/main",       // ind_Latn
    "https://huggingface.co/Sams200/opus-mt-is-en/resolve/main",       // isl_Latn
    "https://huggingface.co/Sams200/opus-mt-it-en/resolve/main",       // ita_Latn
    NULL,                                                               // jav_Latn
    "https://huggingface.co/Sams200/opus-mt-ja-en/resolve/main",        // jpn_Jpan
    NULL,                                                               // kab_Latn
    NULL,                                                               // kac_Latn
    NULL,                                                               // kam_Latn
    NULL,                                                               // kan_Knda
    NULL,                                                               // kas_Arab
    NULL,                                                               // kas_Deva
    NULL,                                                               // kat_Geor
    NULL,                                                               // kaz_Cyrl
    NULL,                                                               // kbp_Latn
    NULL,                                                               // kea_Latn
    NULL,                                                               // khm_Khmr
    NULL,                                                               // kik_Latn
    NULL,                                                               // kin_Latn
    NULL,                                                               // kir_Cyrl
    NULL,                                                               // kmb_Latn
    NULL,                                                               // kmr_Latn
    NULL,                                                               // knc_Arab
    NULL,                                                               // knc_Latn
    NULL,                                                               // kon_Latn
    NULL,                                                               // kor_Hang
    NULL,                                                               // lao_Laoo
    NULL,                                                               // lij_Latn
    NULL,                                                               // lim_Latn
    NULL,                                                               // lin_Latn
    NULL,                                                               // lit_Latn
    NULL,                                                               // lmo_Latn
    NULL,                                                               // ltg_Latn
    NULL,                                                               // ltz_Latn
    NULL,                                                               // lua_Latn
    NULL,                                                               // lug_Latn
    NULL,                                                               // luo_Latn
    NULL,                                                               // lus_Latn
    NULL,                                                               // lvs_Latn
    NULL,                                                               // mag_Deva
    NULL,                                                               // mai_Deva
    NULL,                                                               // mal_Mlym
    NULL,                                                               // mar_Deva
    NULL,                                                               // min_Arab
    NULL,                                                               // min_Latn
    "https://huggingface.co/Sams200/opus-mt-mk-en/resolve/main",       // mkd_Cyrl
    "https://huggingface.co/Sams200/opus-mt-mt-en/resolve/main",       // mlt_Latn
    NULL,                                                               // mni_Beng
    NULL,                                                               // mos_Latn
    NULL,                                                               // mri_Latn
    NULL,                                                               // mya_Mymr
    "https://huggingface.co/Sams200/opus-mt-nl-en/resolve/main",       // nld_Latn
    NULL,                                                               // nno_Latn
    NULL,                                                               // nob_Latn
    NULL,                                                               // npi_Deva
    NULL,                                                               // nso_Latn
    NULL,                                                               // nus_Latn
    NULL,                                                               // nya_Latn
    NULL,                                                               // oci_Latn
    NULL,                                                               // ory_Orya
    NULL,                                                               // pag_Latn
    NULL,                                                               // pan_Guru
    NULL,                                                               // pap_Latn
    NULL,                                                               // pes_Arab
    NULL,                                                               // plt_Latn
    NULL,                                                               // pol_Latn
    NULL,                                                               // por_Latn
    NULL,                                                               // prs_Arab
    NULL,                                                               // pbt_Arab
    NULL,                                                               // quy_Latn
    "https://huggingface.co/Sams200/opus-mt-ro-en/resolve/main",       // ron_Latn
    NULL,                                                               // run_Latn
    "https://huggingface.co/Sams200/opus-mt-ru-en/resolve/main",       // rus_Cyrl
    NULL,                                                               // sag_Latn
    NULL,                                                               // san_Deva
    NULL,                                                               // sat_Olck
    NULL,                                                               // scn_Latn
    NULL,                                                               // shn_Mymr
    NULL,                                                               // sin_Sinh
    "https://huggingface.co/Sams200/opus-mt-sk-en/resolve/main",       // slk_Latn
    NULL,                                                               // slv_Latn
    NULL,                                                               // smo_Latn
    NULL,                                                               // sna_Latn
    NULL,                                                               // snd_Arab
    NULL,                                                               // som_Latn
    NULL,                                                               // sot_Latn
    "https://huggingface.co/Sams200/opus-mt-es-en/resolve/main",       // spa_Latn
    NULL,                                                               // srd_Latn
    NULL,                                                               // srp_Cyrl
    NULL,                                                               // ssw_Latn
    NULL,                                                               // sun_Latn
    "https://huggingface.co/Sams200/opus-mt-sv-en/resolve/main",       // swe_Latn
    "https://huggingface.co/Sams200/opus-mt-sw-en/resolve/main",       // swh_Latn
    NULL,                                                               // szl_Latn
    NULL,                                                               // tam_Taml
    NULL,                                                               // taq_Latn
    NULL,                                                               // taq_Tfng
    NULL,                                                               // tat_Cyrl
    NULL,                                                               // tel_Telu
    NULL,                                                               // tgk_Cyrl
    "https://huggingface.co/Sams200/opus-mt-tl-en/resolve/main",       // tgl_Latn
    NULL,                                                               // tha_Thai
    NULL,                                                               // tir_Ethi
    NULL,                                                               // tpi_Latn
    NULL,                                                               // tsn_Latn
    NULL,                                                               // tso_Latn
    NULL,                                                               // tuk_Latn
    NULL,                                                               // tum_Latn
    NULL,                                                               // tur_Latn
    NULL,                                                               // twi_Latn
    NULL,                                                               // tzm_Tfng
    NULL,                                                               // uig_Arab
    "https://huggingface.co/Sams200/opus-mt-uk-en/resolve/main",       // ukr_Cyrl
    NULL,                                                               // umb_Latn
    "https://huggingface.co/Sams200/opus-mt-ur-en/resolve/main",       // urd_Arab
    NULL,                                                               // uzn_Latn
    NULL,                                                               // vec_Latn
    "https://huggingface.co/Sams200/opus-mt-vi-en/resolve/main",       // vie_Latn
    NULL,                                                               // war_Latn
    NULL,                                                               // wol_Latn
    NULL,                                                               // xho_Latn
    NULL,                                                               // ydd_Hebr
    NULL,                                                               // yor_Latn
    NULL,                                                               // yue_Hant
    "https://huggingface.co/Sams200/opus-mt-zh-en/resolve/main",       // zho_Hans
    NULL,                                                               // zho_Hant
    NULL,                                                               // zsm_Latn
    NULL,                                                               // zul_Latn
    END_OF_ARRAY
};

// Local directory name for the Helsinki en→lang model, aligned with VALID_LANGUAGES.
// NULL = no model for this direction.
static const char *HELSINKI_EN_TO_LANG_DIR[] = {
    NULL,             // ace_Arab
    NULL,             // ace_Latn
    NULL,             // acm_Arab
    NULL,             // acq_Arab
    NULL,             // aeb_Arab
    "opus-mt-en-af",  // afr_Latn
    NULL,             // ajp_Arab
    NULL,             // aka_Latn
    NULL,             // amh_Ethi
    NULL,             // apc_Arab
    "opus-mt-en-ar",  // arb_Arab
    NULL,             // arb_Latn
    NULL,             // ars_Arab
    NULL,             // ary_Arab
    NULL,             // arz_Arab
    NULL,             // asm_Beng
    NULL,             // ast_Latn
    NULL,             // awa_Deva
    NULL,             // ayr_Latn
    NULL,             // azb_Arab
    "opus-mt-en-az",  // azj_Latn
    NULL,             // bak_Cyrl
    NULL,             // bam_Latn
    NULL,             // ban_Latn
    NULL,             // bel_Cyrl
    NULL,             // bem_Latn
    NULL,             // ben_Beng
    NULL,             // bho_Deva
    NULL,             // bjn_Arab
    NULL,             // bjn_Latn
    NULL,             // bod_Tibt
    NULL,             // bos_Latn
    NULL,             // bug_Latn
    "opus-mt-en-bg",  // bul_Cyrl
    "opus-mt-en-ca",  // cat_Latn
    NULL,             // ceb_Latn
    "opus-mt-en-cs",  // ces_Latn
    NULL,             // cjk_Latn
    NULL,             // ckb_Arab
    NULL,             // crh_Latn
    "opus-mt-en-cy",  // cym_Latn
    "opus-mt-en-da",  // dan_Latn
    "opus-mt-en-de",  // deu_Latn
    NULL,             // dik_Latn
    NULL,             // dyu_Latn
    NULL,             // dzo_Tibt
    "opus-mt-en-el",  // ell_Grek
    NULL,             // eng_Latn
    NULL,             // epo_Latn
    "opus-mt-en-et",  // est_Latn
    "opus-mt-en-eu",  // eus_Latn
    NULL,             // ewe_Latn
    "opus-mt-en-fo",  // fao_Latn
    NULL,             // fij_Latn
    "opus-mt-en-fi",  // fin_Latn
    NULL,             // fon_Latn
    "opus-mt-en-fr",  // fra_Latn
    NULL,             // fur_Latn
    NULL,             // fuv_Latn
    NULL,             // gla_Latn
    NULL,             // gle_Latn
    "opus-mt-en-gl",  // glg_Latn
    NULL,             // grn_Latn
    NULL,             // guj_Gujr
    "opus-mt-en-ht",  // hat_Latn
    "opus-mt-en-ha",  // hau_Latn
    "opus-mt-en-he",  // heb_Hebr
    "opus-mt-en-hi",  // hin_Deva
    NULL,             // hne_Deva
    "opus-mt-en-hr",  // hrv_Latn
    "opus-mt-en-hu",  // hun_Latn
    "opus-mt-en-hy",  // hye_Armn
    NULL,             // ibo_Latn
    NULL,             // ilo_Latn
    "opus-mt-en-id",  // ind_Latn
    "opus-mt-en-is",  // isl_Latn
    "opus-mt-en-it",  // ita_Latn
    NULL,             // jav_Latn
    "opus-mt-en-ja",  // jpn_Jpan
    NULL,             // kab_Latn
    NULL,             // kac_Latn
    NULL,             // kam_Latn
    NULL,             // kan_Knda
    NULL,             // kas_Arab
    NULL,             // kas_Deva
    NULL,             // kat_Geor
    NULL,             // kaz_Cyrl
    NULL,             // kbp_Latn
    NULL,             // kea_Latn
    NULL,             // khm_Khmr
    NULL,             // kik_Latn
    NULL,             // kin_Latn
    NULL,             // kir_Cyrl
    NULL,             // kmb_Latn
    NULL,             // kmr_Latn
    NULL,             // knc_Arab
    NULL,             // knc_Latn
    NULL,             // kon_Latn
    "opus-mt-en-ko",  // kor_Hang
    NULL,             // lao_Laoo
    NULL,             // lij_Latn
    NULL,             // lim_Latn
    NULL,             // lin_Latn
    "opus-mt-en-lt",  // lit_Latn
    NULL,             // lmo_Latn
    NULL,             // ltg_Latn
    NULL,             // ltz_Latn
    NULL,             // lua_Latn
    NULL,             // lug_Latn
    NULL,             // luo_Latn
    NULL,             // lus_Latn
    "opus-mt-en-lv",  // lvs_Latn
    NULL,             // mag_Deva
    NULL,             // mai_Deva
    NULL,             // mal_Mlym
    NULL,             // mar_Deva
    NULL,             // min_Arab
    NULL,             // min_Latn
    "opus-mt-en-mk",  // mkd_Cyrl
    "opus-mt-en-mt",  // mlt_Latn
    NULL,             // mni_Beng
    NULL,             // mos_Latn
    NULL,             // mri_Latn
    NULL,             // mya_Mymr
    "opus-mt-en-nl",  // nld_Latn
    NULL,             // nno_Latn
    "opus-mt-en-no",  // nob_Latn
    NULL,             // npi_Deva
    NULL,             // nso_Latn
    NULL,             // nus_Latn
    NULL,             // nya_Latn
    NULL,             // oci_Latn
    NULL,             // ory_Orya
    NULL,             // pag_Latn
    NULL,             // pan_Guru
    NULL,             // pap_Latn
    "opus-mt-en-fa",  // pes_Arab
    NULL,             // plt_Latn
    "opus-mt-en-pl",  // pol_Latn
    "opus-mt-en-pt",  // por_Latn
    NULL,             // prs_Arab
    NULL,             // pbt_Arab
    NULL,             // quy_Latn
    "opus-mt-en-ro",  // ron_Latn
    NULL,             // run_Latn
    "opus-mt-en-ru",  // rus_Cyrl
    NULL,             // sag_Latn
    NULL,             // san_Deva
    NULL,             // sat_Olck
    NULL,             // scn_Latn
    NULL,             // shn_Mymr
    NULL,             // sin_Sinh
    "opus-mt-en-sk",  // slk_Latn
    "opus-mt-en-sl",  // slv_Latn
    NULL,             // smo_Latn
    NULL,             // sna_Latn
    NULL,             // snd_Arab
    NULL,             // som_Latn
    NULL,             // sot_Latn
    "opus-mt-en-es",  // spa_Latn
    NULL,             // srd_Latn
    "opus-mt-en-sr",  // srp_Cyrl
    NULL,             // ssw_Latn
    NULL,             // sun_Latn
    "opus-mt-en-sv",  // swe_Latn
    "opus-mt-en-sw",  // swh_Latn
    NULL,             // szl_Latn
    "opus-mt-en-ta",  // tam_Taml
    NULL,             // taq_Latn
    NULL,             // taq_Tfng
    NULL,             // tat_Cyrl
    NULL,             // tel_Telu
    NULL,             // tgk_Cyrl
    "opus-mt-en-tl",  // tgl_Latn
    "opus-mt-en-th",  // tha_Thai
    NULL,             // tir_Ethi
    NULL,             // tpi_Latn
    NULL,             // tsn_Latn
    NULL,             // tso_Latn
    NULL,             // tuk_Latn
    NULL,             // tum_Latn
    "opus-mt-en-tr",  // tur_Latn
    NULL,             // twi_Latn
    NULL,             // tzm_Tfng
    NULL,             // uig_Arab
    "opus-mt-en-uk",  // ukr_Cyrl
    NULL,             // umb_Latn
    "opus-mt-en-ur",  // urd_Arab
    NULL,             // uzn_Latn
    NULL,             // vec_Latn
    "opus-mt-en-vi",  // vie_Latn
    NULL,             // war_Latn
    NULL,             // wol_Latn
    NULL,             // xho_Latn
    NULL,             // ydd_Hebr
    NULL,             // yor_Latn
    NULL,             // yue_Hant
    "opus-mt-en-zh",  // zho_Hans
    NULL,             // zho_Hant
    "opus-mt-en-ms",  // zsm_Latn
    NULL,             // zul_Latn
    END_OF_ARRAY
};

// Local directory name for the Helsinki lang→en model, aligned with VALID_LANGUAGES.
// NULL = no model for this direction.
static const char *HELSINKI_LANG_TO_EN_DIR[] = {
    NULL,             // ace_Arab
    NULL,             // ace_Latn
    NULL,             // acm_Arab
    NULL,             // acq_Arab
    NULL,             // aeb_Arab
    "opus-mt-af-en",  // afr_Latn
    NULL,             // ajp_Arab
    NULL,             // aka_Latn
    NULL,             // amh_Ethi
    NULL,             // apc_Arab
    "opus-mt-ar-en",  // arb_Arab
    NULL,             // arb_Latn
    NULL,             // ars_Arab
    NULL,             // ary_Arab
    NULL,             // arz_Arab
    NULL,             // asm_Beng
    NULL,             // ast_Latn
    NULL,             // awa_Deva
    NULL,             // ayr_Latn
    NULL,             // azb_Arab
    "opus-mt-az-en",  // azj_Latn
    NULL,             // bak_Cyrl
    NULL,             // bam_Latn
    NULL,             // ban_Latn
    NULL,             // bel_Cyrl
    NULL,             // bem_Latn
    NULL,             // ben_Beng
    NULL,             // bho_Deva
    NULL,             // bjn_Arab
    NULL,             // bjn_Latn
    NULL,             // bod_Tibt
    NULL,             // bos_Latn
    NULL,             // bug_Latn
    "opus-mt-bg-en",  // bul_Cyrl
    "opus-mt-ca-en",  // cat_Latn
    NULL,             // ceb_Latn
    "opus-mt-cs-en",  // ces_Latn
    NULL,             // cjk_Latn
    NULL,             // ckb_Arab
    NULL,             // crh_Latn
    "opus-mt-cy-en",  // cym_Latn
    "opus-mt-da-en",  // dan_Latn
    "opus-mt-de-en",  // deu_Latn
    NULL,             // dik_Latn
    NULL,             // dyu_Latn
    NULL,             // dzo_Tibt
    "opus-mt-el-en",  // ell_Grek
    NULL,             // eng_Latn
    NULL,             // epo_Latn
    "opus-mt-et-en",  // est_Latn
    "opus-mt-eu-en",  // eus_Latn
    NULL,             // ewe_Latn
    "opus-mt-fo-en",  // fao_Latn
    NULL,             // fij_Latn
    "opus-mt-fi-en",  // fin_Latn
    NULL,             // fon_Latn
    "opus-mt-fr-en",  // fra_Latn
    NULL,             // fur_Latn
    NULL,             // fuv_Latn
    NULL,             // gla_Latn
    NULL,             // gle_Latn
    "opus-mt-gl-en",  // glg_Latn
    NULL,             // grn_Latn
    NULL,             // guj_Gujr
    "opus-mt-ht-en",  // hat_Latn
    "opus-mt-ha-en",  // hau_Latn
    "opus-mt-he-en",  // heb_Hebr
    "opus-mt-hi-en",  // hin_Deva
    NULL,             // hne_Deva
    "opus-mt-hr-en",  // hrv_Latn
    "opus-mt-hu-en",  // hun_Latn
    "opus-mt-hy-en",  // hye_Armn
    NULL,             // ibo_Latn
    NULL,             // ilo_Latn
    "opus-mt-id-en",  // ind_Latn
    "opus-mt-is-en",  // isl_Latn
    "opus-mt-it-en",  // ita_Latn
    NULL,             // jav_Latn
    "opus-mt-ja-en",  // jpn_Jpan
    NULL,             // kab_Latn
    NULL,             // kac_Latn
    NULL,             // kam_Latn
    NULL,             // kan_Knda
    NULL,             // kas_Arab
    NULL,             // kas_Deva
    NULL,             // kat_Geor
    NULL,             // kaz_Cyrl
    NULL,             // kbp_Latn
    NULL,             // kea_Latn
    NULL,             // khm_Khmr
    NULL,             // kik_Latn
    NULL,             // kin_Latn
    NULL,             // kir_Cyrl
    NULL,             // kmb_Latn
    NULL,             // kmr_Latn
    NULL,             // knc_Arab
    NULL,             // knc_Latn
    NULL,             // kon_Latn
    "opus-mt-ko-en",  // kor_Hang
    NULL,             // lao_Laoo
    NULL,             // lij_Latn
    NULL,             // lim_Latn
    NULL,             // lin_Latn
    "opus-mt-lt-en",  // lit_Latn
    NULL,             // lmo_Latn
    NULL,             // ltg_Latn
    NULL,             // ltz_Latn
    NULL,             // lua_Latn
    NULL,             // lug_Latn
    NULL,             // luo_Latn
    NULL,             // lus_Latn
    "opus-mt-lv-en",  // lvs_Latn
    NULL,             // mag_Deva
    NULL,             // mai_Deva
    NULL,             // mal_Mlym
    NULL,             // mar_Deva
    NULL,             // min_Arab
    NULL,             // min_Latn
    "opus-mt-mk-en",  // mkd_Cyrl
    "opus-mt-mt-en",  // mlt_Latn
    NULL,             // mni_Beng
    NULL,             // mos_Latn
    NULL,             // mri_Latn
    NULL,             // mya_Mymr
    "opus-mt-nl-en",  // nld_Latn
    NULL,             // nno_Latn
    "opus-mt-no-en",  // nob_Latn
    NULL,             // npi_Deva
    NULL,             // nso_Latn
    NULL,             // nus_Latn
    NULL,             // nya_Latn
    NULL,             // oci_Latn
    NULL,             // ory_Orya
    NULL,             // pag_Latn
    NULL,             // pan_Guru
    NULL,             // pap_Latn
    "opus-mt-fa-en",  // pes_Arab
    NULL,             // plt_Latn
    "opus-mt-pl-en",  // pol_Latn
    "opus-mt-pt-en",  // por_Latn
    NULL,             // prs_Arab
    NULL,             // pbt_Arab
    NULL,             // quy_Latn
    "opus-mt-ro-en",  // ron_Latn
    NULL,             // run_Latn
    "opus-mt-ru-en",  // rus_Cyrl
    NULL,             // sag_Latn
    NULL,             // san_Deva
    NULL,             // sat_Olck
    NULL,             // scn_Latn
    NULL,             // shn_Mymr
    NULL,             // sin_Sinh
    "opus-mt-sk-en",  // slk_Latn
    "opus-mt-sl-en",  // slv_Latn
    NULL,             // smo_Latn
    NULL,             // sna_Latn
    NULL,             // snd_Arab
    NULL,             // som_Latn
    NULL,             // sot_Latn
    "opus-mt-es-en",  // spa_Latn
    NULL,             // srd_Latn
    "opus-mt-sr-en",  // srp_Cyrl
    NULL,             // ssw_Latn
    NULL,             // sun_Latn
    "opus-mt-sv-en",  // swe_Latn
    "opus-mt-sw-en",  // swh_Latn
    NULL,             // szl_Latn
    "opus-mt-ta-en",  // tam_Taml
    NULL,             // taq_Latn
    NULL,             // taq_Tfng
    NULL,             // tat_Cyrl
    NULL,             // tel_Telu
    NULL,             // tgk_Cyrl
    "opus-mt-tl-en",  // tgl_Latn
    "opus-mt-th-en",  // tha_Thai
    NULL,             // tir_Ethi
    NULL,             // tpi_Latn
    NULL,             // tsn_Latn
    NULL,             // tso_Latn
    NULL,             // tuk_Latn
    NULL,             // tum_Latn
    "opus-mt-tr-en",  // tur_Latn
    NULL,             // twi_Latn
    NULL,             // tzm_Tfng
    NULL,             // uig_Arab
    "opus-mt-uk-en",  // ukr_Cyrl
    NULL,             // umb_Latn
    "opus-mt-ur-en",  // urd_Arab
    NULL,             // uzn_Latn
    NULL,             // vec_Latn
    "opus-mt-vi-en",  // vie_Latn
    NULL,             // war_Latn
    NULL,             // wol_Latn
    NULL,             // xho_Latn
    NULL,             // ydd_Hebr
    NULL,             // yor_Latn
    NULL,             // yue_Hant
    "opus-mt-zh-en",  // zho_Hans
    NULL,             // zho_Hant
    "opus-mt-ms-en",  // zsm_Latn
    NULL,             // zul_Latn
    END_OF_ARRAY
};

#endif //DEFAULTS_H
