/**
 * @file
 * BrAPI Javascript library
 *
 */
(function ($/*, Drupal, window, document, undefined*/) {
"use strict";

  Drupal.brapi = Drupal.brapi || {};

  Drupal.behaviors.brapi = {
    attach: function(context, settings) {
/******************************************************************************/
$(function() {

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
  $('form.brapi-autoquery')
    .not('.brapi-processed')
    .addClass('brapi-processed')
    .each(function(index, item) {
      var $brapi_form = $(item);
      var filter_data = {};
      $brapi_form.find('.brapi-query-filter-post input, .brapi-query-filter-post select').each(function(filter_index, filter_item) {
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
        success: function(output) {
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
            // Replace each property place-holder by its value.
            for (var prop in output.result.data[0]) {
              if (output.result.data[0].hasOwnProperty(prop)) {
                var regex = new RegExp('\\[' + prop + '\\]', 'g');
                output_html = output_html.replace(regex, output.result.data[0][prop]);
              }
            }
            $brapi_form.html(decodeURIComponent(output_html));
          }
          else {
            $brapi_form.html('n/a');
          }
        },
        error: function(jqXHR, textStatus, errorThrown) {
          $brapi_form.html('n/a (error)');
        }
      });
    })
  ;

  // Settings.
  $('#brapi_date_settings').on('change', function() {
    switch ($(this).val()) {
      case 'custom':
        $('#brapi_custom_date_format')
          .prop('disabled', false)
          .parent()
            .removeClass('form-disabled');
        break;
      default:
        $('#brapi_custom_date_format')
          .prop('disabled', true)
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
