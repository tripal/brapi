/**
 * @file
 * BrAPI Javascript library.
 */

(function ($) {
"use strict";

  Drupal.brapi = Drupal.brapi || {};

  /**
   * Returns the value or an array of value corresponding to the given path.
   *
   * Parses the given field path to fetch the associated value or values if more
   * than one value correspond to that path. The field path is just a string
   * describing the field and subfields (separated by dots) to follow to acces
   * to the final value. If wildcard (*) are used, every field at that level
   * will be followed. If several values are gathered, they will be returned
   * into an array.
   *
   * @param object data
   *   A Javascript object.
   * @param string field_path
   *   A path to the data field. Wildcard character (*) can be used to match a
   *   set of fields.
   *
   * @return array
   *   An array of values.
   */
  Drupal.brapi.extractDataValues = function (data, field_path) {
console.log("field_path: " + field_path); //+debug
    function brapiGetFieldValue(data_array, field_path_array) {
      var field = field_path_array.shift();
console.log("field: " + field); //+debug
      var new_values = [];
      data_array.forEach(function (current_value) {
        if (field == '*') {
console.log("got a star"); //+debug
          if (Array.isArray(current_value)) {
            new_values = new_values.concat(current_value);
          }
          else if ((typeof current_value === 'object')
                   && (current_value !== null)) {
            for (var key in current_value) {
              if (current_value.hasOwnProperty(key)) {
                new_values.push(current_value[key]);
              }
            }
          }
        }
        else {
console.log("got a value"); //+debug
          new_values.push(current_value[field]);        
        }
      });
      // Last subfield?
      if (0 == field_path_array.length) {
        return new_values;
      }
      else {
        return brapiGetFieldValue(new_values, field_path_array);
      }
    }
    return brapiGetFieldValue([data], field_path.split('.'));
  }

  /**
   * Does the same as extractDataValues() but only return 1 value at most.
   *
   * @param object data
   *   A Javascript object.
   * @param string field_path
   *   A path to the data field. Wildcard character (*) can be used to match a
   *   set of fields.
   *
   * @return mixed
   *   A single value or null.
   */
  Drupal.brapi.extractDataValue = function (data, field_path) {
    var value = Drupal.brapi.extractDataValues(data, field_path);
    if (0 < value.length) {
      return value[0];
    }
    return null;
  }

  /**
   * Process a BrAPI form.
   *
   * Process a BrAPI form through AJAX and replace it with the resulting HTML
   * code.
   *
   * @param object form_node
   *   The BrAPI form HTML element that contains the BrAPI query parameters.
   */
  Drupal.brapi.processForm = function (form_node) {
    var $brapi_form = $(form_node);
    var filter_data = {};
    $brapi_form.find('.brapi-query-filter-post input, .brapi-query-filter-post select').each(function (filter_index, filter_item) {
      if ('' != $(filter_item).val()) {
        filter_data[$(filter_item).attr('name')] = $(filter_item).val();
      }
    });
    if ($.isEmptyObject(filter_data)) {
      filter_data = null;
    }
    else {
      filter_data = JSON.stringify(filter_data, null, '\t');
    }
    // Get from Ajax.
    $.ajax({
      url: $brapi_form.attr('action'),
      type: $brapi_form.attr('method'),
      data: filter_data,
      dataType: 'json',
      success: function (output) {
        if (output
            && output.result
            && output.result.data
            && output.result.data.length) {
          var output_html = $brapi_form.find('input[name="brapi_html"]').val();
          if (!output_html) {
            $brapi_form.find('input').not('.brapi-query-filter-post').remove();
            output_html = encodeURIComponent(
              '<a href="'
              + $brapi_form.attr('action')
              + ($brapi_form.attr('action').match(/\?/) ? '&' : '?')
              + $brapi_form.serialize()
              + '">BrAPI match</a>'
            );
          }
          // @TODO: replace for-loop with new code.
          // Replace each property place-holder by its value.
         // for (var prop in output.result.data[0]) {
         //   if (output.result.data[0].hasOwnProperty(prop)) {
         //     var regex = new RegExp('\\[' + prop + '\\]', 'g');
         //     var value = Drupal.brapi.extractDataValue(output.result.data, prop);
         //     output_html = output_html.replace(regex, value);
         //   }
         // }
          // Create an array of HTML lines with one line.
          var html_lines = [decodeURIComponent(output_html)];
          // Find placeholders.
          var regex_match = new RegExp('\\[([\\w\\.\\*]+)\\]', 'g');
          var placeholders = {};
          var placeholder = regex_match.exec(output_html);
console.log("placeholder: " + placeholder);
          while (placeholder != null) {
console.log("GOT: " + placeholder[1]);
            placeholders[placeholder[1]] = null;
            // Get next placeholder.
            placeholder = regex_match.exec(output_html);
          }
          // For each placeholder, extract corresponding field values.
          for (placeholder in placeholders) {
            placeholders[placeholder] = Drupal.brapi.extractDataValues(
              output.result.data,
              placeholder
            );
            var new_html_lines = [];
            // For HTML each line, duplicate current line for each value and
            // replace the placeholders by the value in the duplicated HTML
            // line.
            html_lines.forEach(function (current_line) {
              var regex_replace = new RegExp(
                '\\['
                + placeholder.replace(/\.|\*/g, function (x) {return '\\' + x;})
                + '\\]',
                'g'
              );
              placeholders[placeholder].forEach(function (current_value) {
                var updated_line =
                  current_line.replace(regex_replace, current_value);
                new_html_lines.push(updated_line);
              });
            });
            // Update HTML lines.
            html_lines = new_html_lines;
          }
          $brapi_form.html(html_lines.join('\n'));
        }
        else {
          $brapi_form.html('n/a');
        }
      },
      error: function (jqXHR, textStatus, errorThrown) {
        $brapi_form.html('n/a (error)');
      }
    });
  }

  Drupal.behaviors.brapi = {
    attach: function (context, settings) {
/******************************************************************************/
$(function () {

  /**
  * Auto-process page BrAPI queries: display matching accession name.
  *
  * A form to auto-process should look like that:
  * @code
  * <form class="brapi-autoquery" action="https://BRAPI_SERVER/brapi/v1/SERVICE?PARAMETERS..." method="GET">
  *   <input type="hidden" name="brapi_html" value="URL_ENCODED_HTML_STRING"/>
  *   <input type="submit" name="submit" value="Get BrAPI data"/>
  * </form>
  * @endcode
  * where "BRAPI_SERVER" is the BrAPI server name, "SERVICE?PARAMETERS..." is
  * the BrAPI service to query with its optional parameters and values and
  * "URL_ENCODED_HTML_STRING" is the URL-encoded HTML code to use to replace
  * the form. In this string, not encoded place-holder string will be replaced
  * by properties of the (first) JSON object returned. A place-holder is a
  * the property name as described in the BrAPI specs inside square-brackets.
  * For instance "[germplasmName]" (for the "germplasm-search" call) will be
  * replace by the germplasm name of the first germplasm returned by the call.
  * Note: array or object properties can not be used here.
  *
  * The form can contain additional call parameters using hidden input or select
  * fields wrapped by an HTML element having the CSS class
  * "brapi-query-filter-post".
  */
  $('form.brapi-query')
    .not('.brapi-processed')
    .addClass('brapi-processed')
    .on('submit', function (event) {
      Drupal.brapi.processForm(this);
      event.preventDefault();
      return false;
    });
  $('form.brapi-autoquery')
    .not('.brapi-processed')
    .addClass('brapi-processed')
    .each(function (index, item) {
      Drupal.brapi.processForm(item);
    });

  // Settings.
  $('#brapi_date_settings').change(function () {
    switch ($(this).val()) {
      case 'custom':
        $('#brapi_custom_date_format')
          .attr('disabled', false)
          .parent()
            .removeClass('form-disabled');
        break;

      default:
        $('#brapi_custom_date_format')
          .attr('disabled', true)
          .parent()
            .addClass('form-disabled');
        break;
    }
  }).change();

});
/******************************************************************************/
    }
  };
})(jQuery);
