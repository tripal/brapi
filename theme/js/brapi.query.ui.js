/**
 * @file
 * BrAPI Javascript library.
 */

(function ($) {
"use strict";

  Drupal.brapi = Drupal.brapi || {};

  Drupal.behaviors.brapi = {
    attach: function (context, settings) {
/******************************************************************************/
$(function () {
  var $settings = $('form#brapi_query_settings');

  /**
   * BrAPI query interface Javascript.
   */

  // Show Ajax elements and hide static ones.
  $('#brapi_query_settings, #brapi_call_settings, #brapi_query_result_ajax')
    .show();
  $('#brapi_query_results, #brapi_query_result_iframe').hide();

  // Manages public BrAPI site URL selection.
  $('#brapi_site_urls')
    .not('.brapi-processed')
    .addClass('brapi-processed')
    .change(function () {
      $('#brapi_query_url').val($(this).val());
    });

  // Simplify interface with a call selection by drop-down.
  var $calls = $('#brapi_query_calls')
    .not('.brapi-processed')
    .addClass('brapi-processed');
  if ($calls.length) {
    // Get call selection drop-down.
    var $select = $('#brapi_call_select')
      .change(function () {
        $('form.brapi-query-call').hide();
        $select.find('option:selected').data('form').show();
      });

    $('form.brapi-query-call').each(function (index, element) {
      var call_name = $(element).find('span[name="call_name"]').first().text();
      var version_classes = $(element).attr('class').match(/brapi-query-api-\w+/g);
      // Root call.
      if ('' == call_name) {
        call_name = '/';
        $('<option value="" class="' + version_classes.join(' ') + '">Root call</option>')
          .appendTo($select)
          .data('form', $(element));
      }
      else {
        $('<option value="' + call_name + '" class="' + version_classes.join(' ') + '">' + call_name + '</option>')
          .appendTo($select)
          .data('form', $(element));
      }
      $(element)
        .hide()
        .submit(function (event) {
          event.preventDefault();
          var $form = $(this);
          // Check and fix URL (make sure we got http/https and no trailing '/' on
          // the service URL).
          var service_url = $('#brapi_query_url').val() + '/' + $select.val();
          if (!service_url.match(/^https?:\/\//i)) {
            alert('Invalid BrAPI Service URL!');
            $('#brapi_query_url').focus();
            return false;
          }
          service_url = service_url.replace(/\/+$/, '');
          // Add URL argument.
          $form.find('input.brapi-query-argument').each(function (index, item) {
            var regex = new RegExp('\{' + $(item).attr('name') + '\}', 'g');
            if (service_url.match(regex)) {
              service_url = service_url.replace(regex, $(item).val());
            }
            else {
              service_url += '/' + $(item).val();
            }
          });
          // Add query string.
          var query_string = $('<form/>').append($form.find('.brapi-query-parameters').clone()).serialize();
          var $pretty = $form.find('input[name="pretty"]');
          if ($pretty.length && !$pretty.is(':checked')) {
            query_string = (query_string ? query_string + '&pretty=0' : 'pretty=0');
          }
          service_url += '?' + query_string;
          $form.find('input.brapi-query-filter-get, select.brapi-query-filter-get').each(function (index, item) {
            if ('' != $(item).val()) {
              service_url += '&' + $(item).attr('name') + '=' + encodeURI($(item).val());
            }
          });
          // Update form action URL.
          $form.attr('action', service_url);
          // Get filter data.
          var filter_data = {};
          $form.find('input.brapi-query-filter-post, select.brapi-query-filter-post').each(function (index, item) {
            if ('' != $(item).val()) {
              filter_data[$(item).attr('name')] = $(item).val();
            }
          });
          if ($.isEmptyObject(filter_data)) {
            filter_data = null;
          }
          else {
            filter_data = JSON.stringify(filter_data, null, '\t');
          }
          // Check for debug mode.
          var $debug = $form.find('input[name="debug"]:checked');
          var $debug_zone = $('#brapi_query_debug');
          if ($debug.length) {
            // Add debug zone if not already there.
            if (!$debug_zone.length) {
              $debug_zone = $('<div id="brapi_query_debug"></div>')
                .insertBefore('#brapi_query_result_title');
              $debug_zone
                .append('<h4 id="brapi_query_debug_title">Data sent</h4>')
                .append('<div id="brapi_query_debug_url"></div>')
                .append('<div id="brapi_query_debug_method"></div>')
                .append('<code></code>');
            }
            $debug_zone.find('#brapi_query_debug_url').html('URL: ' + service_url);
            $debug_zone.find('#brapi_query_debug_method').html('Method: ' + (filter_data ? 'POST' : 'GET'));
            $debug_zone.find('code').html(filter_data);
            $debug_zone.show();
          }
          else {
            $debug_zone.hide();
          }

          // Clear output and display animated gif.
          var brapi_image_path = Drupal.settings.brapi.imagePath;
          $('#brapi_query_results').show();
          $('#brapi_query_result_ajax')
            .html('<img src="' + brapi_image_path + 'loading_icon.gif" title="Loading, please wait..." alt="Loading..."/>');
          // Replace submit by an AJAX call.
          $.ajax({
            url: service_url,
            type: filter_data ? 'POST' : 'GET',
            data: filter_data,
            dataType: 'text',
            success: function (output) {
              $('#brapi_query_result_ajax').html(output.trim());
            },
            error: function (jqXHR, textStatus, errorThrown) {
              if ('application/json' == jqXHR.getResponseHeader('Content-Type')) {
                var brapi_error = JSON.parse(jqXHR.responseText);
                var brapi_error_massage = '';
                brapi_error.metadata.status.forEach(function (element) {
                  brapi_error_massage += element.code + ': ' + element.message + "\n";
                });
                alert("BrAPI call failed:\n" + brapi_error_massage);
              }
              else {
                alert('Failed to run BrAPI call: ' + textStatus + "\n" + errorThrown);
              }
              $('#brapi_query_result_ajax').html(
                'HTTP status code '
                + jqXHR.status
                + ': '
                + jqXHR.statusText
                + "\n\nContent type: "
                + jqXHR.getResponseHeader('Content-Type')
                + "\n\n"
                + jqXHR.responseText
              );
              console.log('All headers:');
              console.log(jqXHR.getAllResponseHeaders());
            }
          });
        });

    });

    // Enable active call form.
    $select.find('option:selected').data('form').show();

    // Make all common fields act like one field...
    // First gather all input names.
    var checkbox_names = {}, text_names = {};
    $('#brapi_query_calls input').each(function (index, element) {
      if ('checkbox' == $(element).attr('type')) {
        checkbox_names[$(element).attr('name')] = true;
      }
      else if ('text' == $(element).attr('type')) {
        text_names[$(element).attr('name')] = true;
      }
    });
    // Then loop on names and makes them act as one field.
    for (var checkbox_name in checkbox_names) {
      $('input[name="' + checkbox_name + '"]').change(function () {
        $('input[name="' + $(this).attr('name') + '"]')
        .attr('checked', $(this).is(':checked'));
      });
    }
    for (var text_name in text_names) {
      $('input[name="' + text_name + '"]').change(function () {
        $('input[name="' + $(this).attr('name') + '"]').val($(this).val());
      });
    }

    // Manage API versions.
    var $version_select = $('#brapi_api_version')
      .change(function () {
        $('fieldset.brapi-query-filters, #brapi_call_select option').hide();
        $(
          'fieldset.brapi-query-api-'
          + $(this).val()
          + ', option.brapi-query-api-'
          + $(this).val())
        .show();
      });
    $('fieldset.brapi-query-filters').hide();
    $('fieldset.brapi-query-api-' + $version_select.val()).show();

  }

  // Setup date picker widgets.
  if ($().datepicker) {
    $("#brapi_query_calls input.brapi-datepicker")
      .not('.brapi-processed')
      .addClass('brapi-processed')
      .datepicker({dateFormat: 'yy-mm-dd'}); // ISO_8601.
  }

});
/******************************************************************************/
    }
  };
})(jQuery);
