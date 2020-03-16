/**
 * @file ttfparser.h
 *
 * A C API for the Rust's ttf-parser library.
 */

#ifndef TTFP_H
#define TTFP_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define TTFP_MAJOR_VERSION 0
#define TTFP_MINOR_VERSION 4
#define TTFP_PATCH_VERSION 0
#define TTFP_VERSION "0.4.0"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief An opaque pointer to the font structure.
 */
typedef struct ttfp_font ttfp_font;

/**
 * @brief A tag type.
 */
typedef uint32_t ttfp_tag;

/**
 * @brief A glyph's tight bounding box.
 */
typedef struct ttfp_bbox {
    int16_t x_min;
    int16_t y_min;
    int16_t x_max;
    int16_t y_max;
} ttfp_bbox;

/**
 * @brief A line metrics.
 */
typedef struct ttfp_line_metrics {
    int16_t position;
    int16_t thickness;
} ttfp_line_metrics;

/**
 * @brief A script metrics.
 */
typedef struct ttfp_script_metrics {
    int16_t x_size;
    int16_t y_size;
    int16_t x_offset;
    int16_t y_offset;
} ttfp_script_metrics;

/**
 * @brief A name record.
 *
 * https://docs.microsoft.com/en-us/typography/opentype/spec/name#name-records
 */
typedef struct ttfp_name_record {
    uint16_t platform_id;
    uint16_t encoding_id;
    uint16_t language_id;
    uint16_t name_id;
    uint16_t name_size;
} ttfp_name_record;

/**
 * @brief A variation axis.
 */
typedef struct ttfp_variation_axis {
    ttfp_tag tag;
    float min_value;
    float def_value;
    float max_value;
    uint16_t name_id;
    bool hidden;
} ttfp_variation_axis;

/**
 * @brief An outline building interface.
 */
typedef struct ttfp_outline_builder {
    void (*move_to)(float x, float y, void *data);
    void (*line_to)(float x, float y, void *data);
    void (*quad_to)(float x1, float y1, float x, float y, void *data);
    void (*curve_to)(float x1, float y1, float x2, float y2, float x, float y, void *data);
    void (*close_path)(void *data);
} ttfp_outline_builder;

/**
 * @brief A list of supported tables.
 */
typedef enum ttfp_table_name {
    TTFP_TABLE_AXIS_VARIATIONS = 0,
    TTFP_TABLE_CHARACTER_TO_GLYPH_INDEX_MAPPING,
    TTFP_TABLE_COMPACT_FONT_FORMAT,
    TTFP_TABLE_COMPACT_FONT_FORMAT_2,
    TTFP_TABLE_FONT_VARIATIONS,
    TTFP_TABLE_GLYPH_DATA,
    TTFP_TABLE_GLYPH_DEFINITION,
    TTFP_TABLE_GLYPH_POSITIONING,
    TTFP_TABLE_GLYPH_SUBSTITUTION,
    TTFP_TABLE_GLYPH_VARIATIONS,
    TTFP_TABLE_HEADER,
    TTFP_TABLE_HORIZONTAL_HEADER,
    TTFP_TABLE_HORIZONTAL_METRICS,
    TTFP_TABLE_HORIZONTAL_METRICS_VARIATIONS,
    TTFP_TABLE_INDEX_TO_LOCATION,
    TTFP_TABLE_KERNING,
    TTFP_TABLE_MAXIMUM_PROFILE,
    TTFP_TABLE_METRICS_VARIATIONS,
    TTFP_TABLE_NAMING,
    TTFP_TABLE_POST_SCRIPT,
    TTFP_TABLE_VERTICAL_HEADER,
    TTFP_TABLE_VERTICAL_METRICS,
    TTFP_TABLE_VERTICAL_METRICS_VARIATIONS,
    TTFP_TABLE_VERTICAL_ORIGIN,
    TTFP_TABLE_WINDOWS_METRICS,
} ttfp_table_name;

/**
 * @brief A list of glyph classes.
 */
typedef enum ttfp_glyph_class {
    TTFP_GLYPH_CLASS_UNKNOWN = 0,
    TTFP_GLYPH_CLASS_BASE,
    TTFP_GLYPH_CLASS_LIGATURE,
    TTFP_GLYPH_CLASS_MARK,
    TTFP_GLYPH_CLASS_COMPONENT,
} ttfp_glyph_class;

/**
 * @brief Initializes the library log.
 *
 * Use it if you want to see any warnings.
 *
 * Will do nothing when library is built without the \b logging feature.
 *
 * All warnings will be printed to the \b stderr.
 */
void ttfp_init_log();

/**
 * @brief Returns the number of fonts stored in a TrueType font collection.
 *
 * @param data The font data.
 * @param data_size The size of the font data.
 * @return Number of fonts or -1 when provided data is not a TrueType font collection
 *         or when number of fonts is larger than INT_MAX.
 */
int ttfp_fonts_in_collection(const uint8_t *data, size_t data_size);

/**
 * @brief Creates a new font parser.
 *
 * This is the only heap allocation in the library.
 *
 * @param data The font data. Must outlive the \b ttfp_font.
 * @param data_size The size of the font data.
 * @param index The font index in collection (typically *.ttc). 0 should be used for basic fonts.
 * @return Font handle or NULL on error.
 */
ttfp_font* ttfp_create_font(const uint8_t *data, size_t data_size, uint32_t index);

/**
 * @brief Destroys the #ttfp_font.
 */
void ttfp_destroy_font(ttfp_font *font);

/**
 * @brief Checks that font has a specified table.
 *
 * Will return `true` only for tables that were successfully parsed.
 */
bool ttfp_has_table(const ttfp_font *font, ttfp_table_name name);

/**
 * @brief Resolves a Glyph ID for a code point.
 *
 * All subtable formats except Mixed Coverage (8) are supported.
 *
 * @param codepoint A valid Unicode codepoint. Otherwise 0 will be returned.
 * @return Returns 0 when glyph is not present or parsing is failed.
 */
uint16_t ttfp_get_glyph_index(const ttfp_font *font, uint32_t codepoint);

/**
 * @brief Resolves a variation of a Glyph ID from two code points.
 *
 * Implemented according to
 * [Unicode Variation Sequences](
 * https://docs.microsoft.com/en-us/typography/opentype/spec/cmap#format-14-unicode-variation-sequences).
 *
 * @param codepoint A valid Unicode codepoint. Otherwise 0 will be returned.
 * @param variation A valid Unicode codepoint. Otherwise 0 will be returned.
 * @return Returns 0 when glyph is not present or parsing is failed.
 */
uint16_t ttfp_get_glyph_var_index(const ttfp_font *font, uint32_t codepoint, uint32_t variation);

/**
 * @brief Returns glyph's horizontal advance.
 *
 * @param glyph_id A glyph ID.
 * @return Glyph's horizontal advance or 0 when not set.
 */
uint16_t ttfp_get_glyph_hor_advance(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns glyph's horizontal side bearing.
 *
 * @param glyph_id A glyph ID.
 * @return Glyph's horizontal side bearing or 0 when not set.
 */
int16_t ttfp_get_glyph_hor_side_bearing(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns glyph's vertical advance.
 *
 * @param glyph_id A glyph ID.
 * @return Glyph's vertical advance or 0 when not set.
 */
uint16_t ttfp_get_glyph_ver_advance(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns glyph's vertical side bearing.
 *
 * @param glyph_id A glyph ID.
 * @return Glyph's vertical side bearing or 0 when not set.
 */
int16_t ttfp_get_glyph_ver_side_bearing(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns glyph's vertical origin.
 *
 * @param glyph_id A glyph ID.
 * @return Glyph's vertical origin or 0 when not set.
 */
int16_t ttfp_get_glyph_y_origin(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns a glyphs pair kerning.
 *
 * Only a horizontal kerning is supported.
 *
 * @param glyph_id1 First glyph ID.
 * @param glyph_id1 Second glyph ID.
 * @return A kerning offset or 0 otherwise.
 */
int16_t ttfp_get_glyphs_kerning(const ttfp_font *font, uint16_t glyph_id1, uint16_t glyph_id2);

/**
 * @brief Returns glyph's name.
 *
 * A glyph name cannot be larger than 255 bytes + 1 byte for '\0'.
 *
 * @param glyph_id A glyph ID.
 * @param name A char buffer longer than 256 bytes.
 * @return \b true on success.
 */
bool ttfp_get_glyph_name(const ttfp_font *font, uint16_t glyph_id, char *name);

/**
 * @brief Returns glyph's class according to Glyph Class Definition Table.
 *
 * @return A glyph class or TTFP_GLYPH_CLASS_UNKNOWN otherwise.
 */
ttfp_glyph_class ttfp_get_glyph_class(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns glyph's mark attachment class according to Mark Attachment Class Definition Table.
 *
 * @return All glyphs not assigned to a class fall into Class 0.
 */
uint16_t ttfp_get_glyph_mark_attachment_class(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Checks that glyph is a mark according to Mark Glyph Sets Table.
 */
bool ttfp_is_mark_glyph(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns the number of name records in the font.
 */
uint16_t ttfp_get_name_records_count(const ttfp_font *font);

/**
 * @brief Returns a name record.
 *
 * @param Record's index. The total count can be obtained via #ttfp_get_name_records_count.
 * @return \b false when \b index is out of range or \b platform_id is invalid.
 */
bool ttfp_get_name_record(const ttfp_font *font, uint16_t index, ttfp_name_record *record);

/**
 * @brief Returns a name record's string.
 *
 * @param name A string buffer that will be filled with the record's name.
 *             Remember that a name will use encoding specified in \b ttfp_name_record.encoding_id
 *             Because of that, the name will not be null-terminated.
 * @param name_size Size of the string buffer. Must be equal to \b ttfp_name_record.name_sizeq
 * @return \b false when \b index is out of range or string buffer is not equal
 *         \b ttfp_name_record.name_size.
 */
bool ttfp_get_name_record_string(const ttfp_font *font, uint16_t index, char *name, size_t name_size);

/**
 * @brief Returns font's units per EM.
 *
 * @return A valid value in a 16..16384 range or 0 otherwise.
 */
uint16_t ttfp_get_units_per_em(const ttfp_font *font);

/**
 * @brief Returns font's ascender value.
 *
 * This function never fails.
 */
int16_t ttfp_get_ascender(const ttfp_font *font);

/**
 * @brief Returns font's descender value.
 *
 * This function never fails.
 */
int16_t ttfp_get_descender(const ttfp_font *font);

/**
 * @brief Returns font's height.
 *
 * This function never fails.
 */
int16_t ttfp_get_height(const ttfp_font *font);

/**
 * @brief Returns font's line gap.
 *
 * This function never fails.
 */
int16_t ttfp_get_line_gap(const ttfp_font *font);

/**
 * @brief Checks that font is marked as \b Regular.
 */
bool ttfp_is_regular(const ttfp_font *font);

/**
 * @brief Checks that font is marked as \b Italic.
 */
bool ttfp_is_italic(const ttfp_font *font);

/**
 * @brief Checks that font is marked as \b Bold.
 */
bool ttfp_is_bold(const ttfp_font *font);

/**
 * @brief Checks that font is marked as \b Oblique.
 */
bool ttfp_is_oblique(const ttfp_font *font);

/**
 * @brief Returns font's weight.
 *
 * @return Returns \b 400 / Normal when OS/2 table is not present.
 */
uint16_t ttfp_get_weight(const ttfp_font *font);

/**
 * @brief Returns font's width.
 *
 * @return A number in a 1..9 range. Returns \b 5 / Normal when OS/2 table is not present.
 */
uint16_t ttfp_get_width(const ttfp_font *font);

/**
 * @brief Returns font's x height.
 *
 * @return Font's x height or 0 when OS/2 table is not present.
 */
int16_t ttfp_get_x_height(const ttfp_font *font);

/**
 * @brief Returns font's underline metrics.
 *
 * @return \b false when the \b post table is not present.
 */
bool ttfp_get_underline_metrics(const ttfp_font *font, ttfp_line_metrics *metrics);

/**
 * @brief Returns font's strikeout metrics.
 *
 * @return \b false when the \b OS/2 table is not present.
 */
bool ttfp_get_strikeout_metrics(const ttfp_font *font, ttfp_line_metrics *metrics);

/**
 * @brief Returns font's subscript metrics.
 *
 * @return \b false when the \b OS/2 table is not present.
 */
bool ttfp_get_subscript_metrics(const ttfp_font *font, ttfp_script_metrics *metrics);

/**
 * @brief Returns font's superscript metrics.
 *
 * @return \b false when the \b OS/2 table is not present.
 */
bool ttfp_get_superscript_metrics(const ttfp_font *font, ttfp_script_metrics *metrics);

/**
 * @brief Returns a total number of glyphs in the font.
 *
 * This function never fails.
 *
 * @return The number of glyphs which is never zero.
 */
uint16_t ttfp_get_number_of_glyphs(const ttfp_font *font);

/**
 * @brief Outlines a glyph using provided outline builder and returns its tight bounding box.
 *
 * \b Warning: since \b ttf-parser is a pull parser,
 * #ttfp_outline_builder will emit segments even when outline is partially malformed.
 * You must check #ttfp_outline_glyph result for error before using
 * #ttfp_outline_builder's output.
 *
 * This method supports \b glyf, \b CFF and \b CFF2 tables.
 */
bool ttfp_outline_glyph(const ttfp_font *font,
                        ttfp_outline_builder builder,
                        void* user_data,
                        uint16_t glyph_id,
                        ttfp_bbox *bbox);

/**
 * @brief Outlines a variable glyph and returns its tight bounding box.
 *
 * \b coordinates should be represented in a -1.0..1.0 range using fixed point 2.14.
 * i.e. the float value should be multiplied by 16384.
 *
 * Number of \b coordinates should be the same as the number of variation axes in the font.
 *
 * \b Warning: since \b ttf-parser is a pull parser,
 * #ttfp_outline_builder will emit segments even when outline is partially malformed.
 * You must check #ttfp_outline_variable_glyph result for error before using
 * #ttfp_outline_builder's output.
 *
 * This method supports \b glyf + \b gvar and \b CFF2 tables.
 */
bool ttfp_outline_variable_glyph(ttfp_font *font,
                                 ttfp_outline_builder builder,
                                 void* user_data,
                                 uint16_t glyph_id,
                                 const int16_t *coordinates,
                                 uint32_t coordinates_size,
                                 ttfp_bbox *bbox);

/**
 * @brief Returns a tight glyph bounding box.
 *
 * Note that this method's performance depends on a table type the current font is using.
 * In case of a \b glyf table, it's basically free, since this table stores
 * bounding box separately. In case of `CFF` we should actually outline
 * a glyph and then calculate its bounding box. So if you need an outline and
 * a bounding box and you have an OpenType font (which uses CFF)
 * then prefer #ttfp_outline_glyph method.
 */
bool ttfp_get_glyph_bbox(const ttfp_font *font,
                         uint16_t glyph_id,
                         ttfp_bbox *bbox);

/**
 * @brief Returns a number of variation axes.
 */
uint16_t ttfp_variation_axes_count(const ttfp_font *font);

/**
 * @brief Returns a variation axis by index.
 */
bool ttfp_get_variation_axis(const ttfp_font *font,
                             uint16_t index,
                             ttfp_variation_axis *axis);

/**
 * @brief Returns a variation axis by tag.
 */
bool ttfp_get_variation_axis_by_tag(const ttfp_font *font,
                                    ttfp_tag tag,
                                    ttfp_variation_axis *axis);

/**
 * @brief Performs normalization mapping to variation coordinates.
 *
 * Note: coordinates should be converted from fixed point 2.14 to int16_t
 * by multiplying each coordinate by 16384.
 *
 * Number of \b coordinates should be the same as number of variation axes in the font.
 */
bool ttfp_map_variation_coordinates(const ttfp_font *font,
                                    int16_t *coordinates,
                                    uint32_t coordinates_size);

#ifdef __cplusplus
}
#endif

#endif /* TTFP_H */
