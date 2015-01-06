<?php
/**
 * @file
 * Helper function to clean up strings.
 */

class StringCleanUp {
  /**
   * Deal with encodings.
   *
   * @param string $string
   *   The string to have its encoding processed.
   *
   * @return string
   *   The processed string.
   */
  public static function fixEncoding($string = '') {

    // Fix and replace bizarre characters pre-encoding.
    $string = StringCleanUp::convertFatalCharstoASCII($string);

    // If the content is not UTF8, attempt to convert it.  If encoding can't be
    // detected, then it can't be converted.
    $type_detect = array(
      'UTF-8',
      'ASCII',
      'ISO-8859-1',
      'ISO-8859-2',
      'ISO-8859-3',
      'ISO-8859-4',
      'ISO-8859-5',
      'ISO-8859-6',
      'ISO-8859-7',
      'ISO-8859-8',
      'ISO-8859-9',
      'ISO-8859-10',
      'ISO-8859-13',
      'ISO-8859-14',
      'ISO-8859-15',
      'ISO-8859-16',
      'Windows-1251',
      'Windows-1252',
      'Windows-1254',
    );
    $encoding = mb_detect_encoding($string, $type_detect, TRUE);
    $is_utf8 = mb_check_encoding($string, 'UTF-8');

    if (!$is_utf8 && !empty($encoding)) {
      $string = mb_convert_encoding($string, 'UTF-8', $encoding);
    }

    // Fix and replace bizarre characters to get those caused by encoding.
    $string = StringCleanUp::convertFatalCharstoASCII($string);

    // @TODO Here would be the spot to run a diff comparing before and after
    // encoding and then watchdog the offending character that results in �.
    // Then the offending character can be added to fatalCharsMap().

    return $string;
  }

  /**
   * Map of fatal chars to the replacements.
   *
   * @return array
   *   An array with the mappings.
   */
  public static function fatalCharsMap() {
    $convert_table = array(
      '°' => '&deg;',
      '¡' => '&iexcl;', '&#xa1;' => '&iexcl;',
      '¿' => '&iquest;', '&#xbf;' => '&iquest;',
      'á' => '&agrave;', 'Á' => '&Aacute;', '&#xc4;' => '&Aacute;', '&#xe1;' => '&aacute;',
      'é' => '&eacute;', 'É' => '&Eacute;', '&#xc9;' => '&Eacute;', '&#xe9;' => '&eacute;',
      'Í' => '&Iacute;', 'í' => '&iacute;', '&#xcd;' => '&Iacute;', '&#xed;' => '&iacute;',
      'ó' => '&oacute;', '&#xF3;' => '&oacute;','&#xf3;' => '&oacute;', '&#xd3;' => '&Oacute;', 'Ó' => '&Oacute;',
      'ú' => '&uacute;', '&#xfa;' => '&uacute;', 'Ú' => '&Uacute;', '&#xda;' => '&Uacute;',
      'ü' => '&uuml;', '&#xfc;' => '&uuml;', 'Ü' => '&Uuml;', '&#xdc;' => '&Uuml;',
      'ñ' => '&ntilde;', '&#xf1;' => '&ntilde;',  'Ñ' => '&Ntilde;', '&#xd1;' => '&Ntilde;', '&#xF1;' => '&ntilde;',
    );

    return $convert_table;
  }

  /**
   * Take all things that are not digits or the alphabet and simplify it.
   *
   * This should get rid of most accents, and language specific chars.
   *
   * @param string $string
   *   A string.
   *
   * @return string
   *   The converted string.
   */
  public static function convertFatalCharstoASCII($string = '') {

    foreach (StringCleanUp::fatalCharsMap() as $weird => $normal) {
      $string = str_replace($weird, $normal, $string);
    }

    return $string;
  }

  /**
   * Map of unconventional chars to there some what equivalents.
   *
   * @return array
   *   An array with the mappings.
   */
  public static function funkyCharsMap() {
    $convert_table = array(
      '©' => 'c', '®' => 'r', 'À' => 'a', 'Ã' => 'a',
      'Á' => 'a', 'Â' => 'a', 'Ä' => 'a', 'Å' => 'a', 'Æ' => 'ae','Ç' => 'c',
      'È' => 'e', 'É' => 'e', 'Ë' => 'e', 'Ì' => 'i', 'Í' => 'i', 'Î' => 'i',
      'Ï' => 'i', 'Ò' => 'o', 'Ó' => 'o', 'Ô' => 'o', 'Õ' => 'o', 'Ö' => 'o',
      'Ø' => 'o', 'Ù' => 'u', 'Ú' => 'u', 'Û' => 'u', 'Ü' => 'u', 'Ý' => 'y',
      'ß' => 'ss','à' => 'a', 'á' => 'a', 'â' => 'a', 'ä' => 'a', 'å' => 'a',
      'æ' => 'ae','ç' => 'c', 'è' => 'e', 'é' => 'e', 'ê' => 'e', 'ë' => 'e',
      'ì' => 'i', 'í' => 'i', 'î' => 'i', 'ï' => 'i', 'ò' => 'o', 'ó' => 'o',
      'ô' => 'o', 'õ' => 'o', 'ö' => 'o', 'ø' => 'o', 'ù' => 'u', 'ú' => 'u',
      'û' => 'u', 'ü' => 'u', 'ý' => 'y', 'þ' => 'p', 'ÿ' => 'y', 'Ā' => 'a',
      'ā' => 'a', 'Ă' => 'a', 'ă' => 'a', 'Ą' => 'a', 'ą' => 'a', 'Ć' => 'c',
      'ć' => 'c', 'Ĉ' => 'c', 'ĉ' => 'c', 'Ċ' => 'c', 'ċ' => 'c', 'Č' => 'c',
      'č' => 'c', 'Ď' => 'd', 'ď' => 'd', 'Đ' => 'd', 'đ' => 'd', 'Ē' => 'e',
      'ē' => 'e', 'Ĕ' => 'e', 'ĕ' => 'e', 'Ė' => 'e', 'ė' => 'e', 'Ę' => 'e',
      'ę' => 'e', 'Ě' => 'e', 'ě' => 'e', 'Ĝ' => 'g', 'ĝ' => 'g', 'Ğ' => 'g',
      'ğ' => 'g', 'Ġ' => 'g', 'ġ' => 'g', 'Ģ' => 'g', 'ģ' => 'g', 'Ĥ' => 'h',
      'ĥ' => 'h', 'Ħ' => 'h', 'ħ' => 'h', 'Ĩ' => 'i', 'ĩ' => 'i', 'Ī' => 'i',
      'ī' => 'i', 'Ĭ' => 'i', 'ĭ' => 'i', 'Į' => 'i', 'į' => 'i', 'İ' => 'i',
      'ı' => 'i', 'Ĳ' => 'ij','ĳ' => 'ij','Ĵ' => 'j', 'ĵ' => 'j', 'Ķ' => 'k',
      'ķ' => 'k', 'ĸ' => 'k', 'Ĺ' => 'l', 'ĺ' => 'l', 'Ļ' => 'l', 'ļ' => 'l',
      'Ľ' => 'l', 'ľ' => 'l', 'Ŀ' => 'l', 'ŀ' => 'l', 'Ł' => 'l', 'ł' => 'l',
      'Ń' => 'n', 'ń' => 'n', 'Ņ' => 'n', 'ņ' => 'n', 'Ň' => 'n', 'ň' => 'n',
      'ŉ' => 'n', 'Ŋ' => 'n', 'ŋ' => 'n', 'ñ' => 'n', 'Ō' => 'o', 'ō' => 'o',
      'Ŏ' => 'o', 'ŏ' => 'o', 'Ő' => 'o', 'ő' => 'o', 'Œ' => 'oe','œ' => 'oe',
      'Ŕ' => 'r', 'ŕ' => 'r', 'Ŗ' => 'r', 'ŗ' => 'r', 'Ř' => 'r', 'ř' => 'r',
      'Ś' => 's', 'ś' => 's', 'Ŝ' => 's', 'ŝ' => 's', 'Ş' => 's', 'ş' => 's',
      'Š' => 's', 'š' => 's', 'Ţ' => 't', 'ţ' => 't', 'Ť' => 't', 'ť' => 't',
      'Ŧ' => 't', 'ŧ' => 't', 'Ũ' => 'u', 'ũ' => 'u', 'Ū' => 'u', 'ū' => 'u',
      'Ŭ' => 'u', 'ŭ' => 'u', 'Ů' => 'u', 'ů' => 'u', 'Ű' => 'u', 'ű' => 'u',
      'Ų' => 'u', 'ų' => 'u', 'Ŵ' => 'w', 'ŵ' => 'w', 'Ŷ' => 'y', 'ŷ' => 'y',
      'Ÿ' => 'y', 'Ź' => 'z', 'ź' => 'z', 'Ż' => 'z', 'ż' => 'z', 'Ž' => 'z',
      'ž' => 'z', 'ſ' => 'z', 'Ə' => 'e', 'ƒ' => 'f', 'Ơ' => 'o', 'ơ' => 'o',
      'Ư' => 'u', 'ư' => 'u', 'Ǎ' => 'a', 'ǎ' => 'a', 'Ǐ' => 'i', 'ǐ' => 'i',
      'Ǒ' => 'o', 'ǒ' => 'o', 'Ǔ' => 'u', 'ǔ' => 'u', 'Ǖ' => 'u', 'ǖ' => 'u',
      'Ǘ' => 'u', 'ǘ' => 'u', 'Ǚ' => 'u', 'ǚ' => 'u', 'Ǜ' => 'u', 'ǜ' => 'u',
      'Ǻ' => 'a', 'ǻ' => 'a', 'Ǽ' => 'ae','ǽ' => 'ae','Ǿ' => 'o', 'ǿ' => 'o',
      'ə' => 'e', 'Ё' => 'jo','Є' => 'e', 'І' => 'i', 'Ї' => 'i', 'А' => 'a',
      'Б' => 'b', 'В' => 'v', 'Г' => 'g', 'Д' => 'd', 'Е' => 'e', 'Ж' => 'zh',
      'З' => 'z', 'И' => 'i', 'Й' => 'j', 'К' => 'k', 'Л' => 'l', 'М' => 'm',
      'Н' => 'n', 'О' => 'o', 'П' => 'p', 'Р' => 'r', 'С' => 's', 'Т' => 't',
      'У' => 'u', 'Ф' => 'f', 'Х' => 'h', 'Ц' => 'c', 'Ч' => 'ch','Ш' => 'sh',
      'Щ' => 'sch', 'Ъ' => '-', 'Ы' => 'y', 'Ь' => '-', 'Э' => 'je','Ю' => 'ju',
      'Я' => 'ja', 'а' => 'a', 'б' => 'b', 'в' => 'v', 'г' => 'g', 'д' => 'd',
      'е' => 'e', 'ж' => 'zh','з' => 'z', 'и' => 'i', 'й' => 'j', 'к' => 'k',
      'л' => 'l', 'м' => 'm', 'н' => 'n', 'о' => 'o', 'п' => 'p', 'р' => 'r',
      'с' => 's', 'т' => 't', 'у' => 'u', 'ф' => 'f', 'х' => 'h', 'ц' => 'c',
      'ч' => 'ch', 'ш' => 'sh','щ' => 'sch','ъ' => '-','ы' => 'y', 'ь' => '-',
      'э' => 'je', 'ю' => 'ju','я' => 'ja','ё' => 'jo','є' => 'e', 'і' => 'i',
      'ї' => 'i', 'Ґ' => 'g', 'ґ' => 'g', 'א' => 'a', 'ב' => 'b', 'ג' => 'g',
      'ד' => 'd', 'ה' => 'h', 'ו' => 'v', 'ז' => 'z', 'ח' => 'h', 'ט' => 't',
      'י' => 'i', 'ך' => 'k', 'כ' => 'k', 'ל' => 'l', 'ם' => 'm', 'מ' => 'm',
      'ן' => 'n', 'נ' => 'n', 'ס' => 's', 'ע' => 'e', 'ף' => 'p', 'פ' => 'p',
      'ץ' => 'C', 'צ' => 'c', 'ק' => 'q', 'ר' => 'r', 'ש' => 'w', 'ת' => 't',
      '™' => 'tm', '°' => 'degree',
    );

    return $convert_table;
  }

  /**
   * Take all things that are not digits or the alphabet and simplify it.
   *
   * This should get rid of most accents, and language specific chars.
   *
   * @param string $string
   *   A string.
   *
   * @return string
   *   The converted string.
   */
  public static function convertNonASCIItoASCII($string = '') {

    foreach (StringCleanUp::funkyCharsMap() as $weird => $normal) {
      $string = str_replace($weird, $normal, $string);
    }

    return $string;
  }

  /**
   * Removes all funky characters from a string.
   *
   * @param string $string
   *   A string.
   *
   * @return string
   *   The converted string.
   */
  public static function stripFunkyChars($string = '') {

    foreach (StringCleanUp::funkyCharsMap() as $weird => $normal) {
      $string = str_replace($weird, '', $string);
    }

    return $string;
  }

  /**
   * Trim string from various types of whitespace.
   *
   * @param string $string
   *   The text string from which to remove whitespace.
   *
   * @return string
   *   The trimmed string.
   */
  public static function superTrim($string) {
    // Remove unicode whitespace
    // @see http://stackoverflow.com/questions/4166896/trim-unicode-whitespace-in-php-5-2
    $string = preg_replace('/^[\pZ\pC]+|[\pZ\pC]+$/u', '', $string);
    return $string;
  }

  /**
   * Strip windows CR characters.
   */
  public static function stripWindowsCRChars($string) {
    // We need to strip the Windows CR characters, because otherwise we end up
    // with &#13; in the output.
    // http://technosophos.com/content/querypath-whats-13-end-every-line
    return str_replace(chr(13), '', $string);
  }

  /**
   * Fix window specific chars.
   *
   * @param string $string
   *   A string.
   *
   * @return string
   *   Fixed string.
   */
  public static function fixWindowSpecificChars($string) {
    // Unicode hex codes for chars only supported in windows.
    $incorrect = array(91, 92, 93, 94, 96, 97);

    // Bad chars reference: http://www.w3schools.com/charsets/ref_html_ansi.asp
    // Good chars reference: http://www.utexas.edu/learn/html/spchar.html
    $map = array(
      145 => 8216,
      146 => 8217,
      147 => 8220,
      148 => 8221,
      150 => 8211,
      151 => 8212,
    );

    // Create unicode chars to match.
    foreach ($incorrect as $hex) {
      $unistring = "\u00{$hex}";
      $char = json_decode('"' . $unistring . '"');
      $dec = hexdec("00{$hex}");
      $html = "&#{$map[$dec]};";
      $string = str_replace($char, $html, $string);
    }

    return $string;
  }
}
