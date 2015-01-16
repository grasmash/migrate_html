<?php

/**
 * @file
 * Miscellaneous utility functions.
 */

/**
 * Converts a geocoded point returned by geocoder into an address array.
 *
 * @param obj $point
 *   A geocoded point returned by geocoder.
 *
 * @return array
 *   An associative array containing the formatter address.
 */
function migrate_html_convert_geocoded_point_to_address($point) {
  $address = array();

  if (!empty($point->data['geocoder_address_components'])) {
    $address_parts = $point->data['geocoder_address_components'];

    foreach ($address_parts as $address_part) {
      if (!empty($address_part->types[0])) {
        $address[$address_part->types[0]] = $address_part->short_name;
      }
    }
  }

  return $address;
}

/**
 * Decodes all HTML entities, including numeric and hexadecimal ones.
 *
 * @param mixed $string
 *   A string.
 *
 * @return string
 *   decoded HTML.
 */
function migrate_html_html_entity_decode_numeric($string, $quote_style = ENT_COMPAT, $charset = "utf-8") {
  $string = html_entity_decode($string, $quote_style, $charset);
  $string = preg_replace_callback('/&#x([0-9a-fA-F]+)/', 'migrate_html_hex_chr_utf8_callback', $string);
  $string = preg_replace_callback('/&#([0-9]+)/', 'migrate_html_chr_utf8_callback', $string);
  return $string;
}

/**
 * Callback helper.
 */
function migrate_html_hex_chr_utf8_callback($matches = array()) {
  return migrate_html_chr_utf8(hexdec($matches[1]));
}

/**
 * Callback helper.
 */
function migrate_html_chr_utf8_callback($matches = array()) {
  return migrate_html_chr_utf8($matches[1]);
}

/**
 * Multi-byte chr(): Will turn a numeric argument into a UTF-8 string.
 *
 * @param mixed $num
 *   A number.
 *
 * @return string
 *   The char represented by the number.
 */
function migrate_html_chr_utf8($num) {
  if ($num < 128) {
    return chr($num);
  }
  if ($num < 2048) {
    return chr(($num >> 6) + 192) . chr(($num & 63) + 128);
  }
  if ($num < 65536) {
    return chr(($num >> 12) + 224) . chr((($num >> 6) & 63) + 128) . chr(($num & 63) + 128);
  }
  if ($num < 2097152) {
    return chr(($num >> 18) + 240) . chr((($num >> 12) & 63) + 128) . chr((($num >> 6) & 63) + 128) . chr(($num & 63) + 128);
  }
  return '';
}

/**
 * Take a legacy uri, and map it to an alias.
 *
 * @param string $legacy_uri
 *   A legacy uri gets redirected to a node.
 *
 * @return string
 *   The alias matching the legacy uri, or an empty sting.
 */
function migrate_html_legacy_to_uri($legacy_uri) {
  $redirect = redirect_load_by_source($legacy_uri);
  if ($redirect) {
    $nid = str_replace("node/", "", $redirect->redirect);
    $node = node_load($nid);
    if ($node && array_key_exists('alias', $node->path) && !empty($node->path['alias'])) {
      return $node->path['alias'];
    }

    if ($node) {
      $uri = entity_uri("node", $node);
      if (array_key_exists('path', $uri)) {
        return $uri['path'];
      }
    }
  }
  watchdog("migrate_html", "legacy uri @legacy_uri does not have a node associated with it", array('legacy_uri' => $legacy_uri), WATCHDOG_ALERT);
  return "";
}

/**
 * Generate a random string.
 *
 * @return string
 *   Random string.
 */
function migrate_html_random_string() {
  $characters = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
  $randstring = '';
  for ($i = 0; $i < 10; $i++) {
    $randstring .= $characters[rand(0, strlen($characters))];
  }
  return $randstring;
}

/**
 * Convert a relative url to absolute.
 *
 * @param string $rel
 *   Relative url.
 * @param string $base
 *   Base url.
 * @param string $subpath
 *   An optional sub-path to check for when translating relative URIs that are
 *   not root based.
 *
 * @return string
 *   The relative url transformed to absolute.
 */
function migrate_html_relative_to_absolute_url($rel, $base, $subpath = '') {
  // Return if already absolute URL.
  if (parse_url($rel, PHP_URL_SCHEME) != '') {
    return $rel;
  }

  // Check for presence of subpath in $rel to see if a subpath is missing.
  if ((!empty($subpath)) && (!stristr($rel, $subpath))) {
    // The subpath is not present, so add it.
    $rel = $subpath . '/' . $rel;
  }

  // Queries and anchors.
  if ($rel[0] == '#' || $rel[0] == '?') {
    return $base . $rel;
  }

  // Parse base URL and convert to local variables:
  // $scheme, $host, $path.
  extract(parse_url($base));

  // Remove non-directory element from path.
  $path = preg_replace('#/[^/]*$#', '', $path);

  // Destroy path if relative url points to root.
  if ($rel[0] == '/') {
    $path = '';
  }

  // Dirty absolute URL.
  $abs = "$host$path/$rel";

  // Replace '//' or '/./' or '/foo/../' with '/'.
  $re = array('#(/\.?/)#', '#/(?!\.\.)[^/]+/\.\./#');
  for ($n = 1; $n > 0; $abs = preg_replace($re, '/', $abs, -1, $n)) {
  }

  // Absolute URL is ready.
  return $scheme . '://' . $abs;
}
