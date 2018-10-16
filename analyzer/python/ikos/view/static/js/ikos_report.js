/** Init the list of checks
 *
 * param e - event
 */
function init_checks(e) {
  for (var line_num in window.checks) {
    var checks = window.checks[line_num];
    var line_wrap = document.getElementById('L' + line_num);

    // Init checks box
    var checks_box = line_wrap.getElementsByClassName('checks')[0];
    for (var i = 0; i < checks.length; i++) {
      var check_line = create_check_line(line_num, checks[i]);
      checks_box.appendChild(check_line);
    }
    checks_box.classList.remove('hidden')

    // Init toggle button
    var toggle = line_wrap.getElementsByClassName('toggle')[0];
    toggle.addEventListener('click', toggle_checks);
    toggle.parentNode.classList.remove('hidden');
  }
}

/** Create a check line
 *
 * param line_num - line number
 * param check - the check
 */
function create_check_line(line_num, check) {
  var check_line = window.template_check.cloneNode(true);

  // Attach check information
  check_line._line_num = line_num;
  check_line._check = check;

  // Update id and class
  check_line.id = '';
  check_line.className = [
    'check',
    'kind_' + check.kind,
    'status_' + check.status
  ].join(' ');

  // Add message
  var message = check_line.getElementsByClassName('message')[0];

  var header = line_num + ':' + check.column + ': ';
  var bold = document.createElement('b');
  bold.appendChild(document.createTextNode(header));
  message.appendChild(bold);

  var text = check.message;
  text = text.replace(/\n/g, "\n" + " ".repeat(header.length) + "\t");
  text = text.replace(/\t/g, "    ");
  message.appendChild(document.createTextNode(text));

  var call_contexts_toggle =
      check_line.getElementsByClassName('call_contexts_toggle')[0];
  call_contexts_toggle.addEventListener('click', toggle_call_contexts_modal);

  return check_line;
}

/** Initialize the list of check kinds
 *
 * param e - event
 */
function init_check_kinds_list(e) {
  var list = document.getElementById('check_kinds_list');
  for (var i = 0; i < window.check_kinds.length; i++) {
    var check_kind = window.check_kinds[i];

    var button = document.createElement('input');
    button.type = 'checkbox';
    button.checked = window.check_kinds_filter[check_kind.id]
    button.value = check_kind.id;
    button.addEventListener('change', function(e) {
      filter_by_check_kind(e);
    });

    var label = document.createElement('label')
    label.appendChild(button);
    label.appendChild(document.createTextNode(check_kind.name));

    var li = document.createElement('li');
    li.appendChild(label);

    list.appendChild(li);

    if (!button.checked) {
      filter_by_check_kind({target: button}); // Initial filter
    }
  }
}

/** Initialize the status checkbox (Ok, Warning, Error, Dead Code)
 *
 * param e - event
 */
function init_status_checkbox(e) {
  var checkboxs = document.getElementsByClassName("checkbox_status");
  for (var i = 0; i < checkboxs.length; i++) {
    var checkbox = checkboxs[i];
    checkbox.addEventListener('change', filter_by_status);

    if (!checkbox.checked) {
      filter_by_status({target: checkbox}); // Initial filter
    }
  }
}

/** Callback function called when the user clicks on a check kinds checkbox
 *
 * param e - event
 */
function filter_by_check_kind(e) {
  var checked = e.target.checked;
  var kind = e.target.value;

  // Show/hide check lines
  var check_lines = document.getElementsByClassName("check kind_" + kind);
  var updated_lines = [];
  for (var i = 0; i < check_lines.length; i++) {
    var check_line = check_lines[i];

    if (checked) {
      check_line.classList.remove('kind_hidden');
    } else {
      check_line.classList.add('kind_hidden');
    }

    updated_lines.push(check_line._line_num);
  }

  // Show/Hide checks boxes
  update_checks_boxes(updated_lines);

  // Update filter
  window.check_kinds_filter[kind] = checked;
}

/** Callback function called when the use clicks on a status checkbox
 *
 * param e - event
 */
function filter_by_status(e) {
  var checked = e.target.checked;
  var status = e.target.value;

  // Show/hide check lines
  var check_lines = document.getElementsByClassName('check status_' + status);
  var updated_lines = [];
  for (var i = 0; i < check_lines.length; i++) {
    var check_line = check_lines[i];

    if (checked) {
      check_line.classList.remove('status_hidden');
    } else {
      check_line.classList.add('status_hidden');
    }

    updated_lines.push(check_line._line_num);
  }

  // Show/Hide checks boxes
  update_checks_boxes(updated_lines);
}

/** Update the 'checks' boxes: display or hide them if every checks inside are
 * hidden, or not.
 *
 * param line_numbers - list of line numbers to update
 */
function update_checks_boxes(line_numbers) {
  // Remove duplicates
  line_numbers = line_numbers.filter(function(item, index, array) {
    return array.indexOf(item) === index;
  });

  for (var i = 0; i < line_numbers.length; i++) {
    var line_num = line_numbers[i];
    var line_wrap = document.getElementById('L' + line_num);
    var checks_box = line_wrap.getElementsByClassName('checks')[0];
    var hidden_checks = checks_box.getElementsByClassName('status_hidden').length
      + checks_box.getElementsByClassName('kind_hidden').length
      - checks_box.getElementsByClassName('status_hidden kind_hidden').length;

    if (hidden_checks === checks_box.childElementCount) {
      // No checks in the checks box, hide it
      checks_box.classList.add('hidden');
    } else {
      // At least one check, show it
      checks_box.classList.remove('hidden');
    }
  }
}

/** Callback function called when the user toggles the checks for a line
 *
 * param e - event
 */
function toggle_checks(e) {
  var checks_box = e.target.parentNode.parentNode.nextElementSibling;

  if (checks_box.classList.contains('hidden')) {
    // Show the checks box
    checks_box.classList.remove('hidden');

    // Show all the check lines
    var check_lines = checks_box.getElementsByClassName("check");
    for (var i = 0; i < check_lines.length; i++) {
      var check_line = check_lines[i];
      check_line.classList.remove('kind_hidden');
      check_line.classList.remove('status_hidden');
    }
  } else {
    // Hide the checks box
    checks_box.classList.add('hidden');
  }
}

var navigators = {
  error: {
    lines: [],
    current: -1
  },
  warning: {
    lines: [],
    current: -1
  },
  deadcode: {
    lines: [],
    current: -1
  },
  container: null
}

/** Initialize the navigator
 *
 * param e - event
 */
function init_navigators(e) {
  window.navigators.error.lines = document.getElementsByClassName('line_wrap status_2');
  window.navigators.warning.lines = document.getElementsByClassName('line_wrap status_1');
  window.navigators.deadcode.lines = document.getElementsByClassName('line_wrap status_3');
  window.navigators.container = document.getElementById('page_content_report');

  // errors
  document.getElementById('nav_error_prev')
    .addEventListener('click', function(e) { move_navigator(e, 'error', -1); });
  document.getElementById('nav_error_next')
    .addEventListener('click', function(e) { move_navigator(e, 'error', 1); });
  if (window.navigators.error.lines.length == 0) {
    document.getElementById('navigator-error').classList.add('hidden');
  }

  // warnings
  document.getElementById('nav_warning_prev')
    .addEventListener('click', function(e) { move_navigator(e, 'warning', -1); });
  document.getElementById('nav_warning_next')
    .addEventListener('click', function(e) { move_navigator(e, 'warning', 1); });
  if (window.navigators.warning.lines.length == 0) {
    document.getElementById('navigator-warning').classList.add('hidden');
  }

  // deadcodes
  document.getElementById('nav_deadcode_prev')
    .addEventListener('click', function(e) { move_navigator(e, 'deadcode', -1); });
  document.getElementById('nav_deadcode_next')
    .addEventListener('click', function(e) { move_navigator(e, 'deadcode', 1); });
  if (window.navigators.deadcode.lines.length == 0) {
    document.getElementById('navigator-deadcode').classList.add('hidden');
  }

  // reset button
  document.getElementById('nav_reset')
    .addEventListener('click', reset_navigator);

  // update labels of buttons
  update_navigator_buttons('error');
  update_navigator_buttons('warning');
  update_navigator_buttons('deadcode');
}

/** Update navigator buttons
 *
 * param status - error or warning
 */
function update_navigator_buttons(status) {
  var current = window.navigators[status].current;
  var lines = window.navigators[status].lines;

  var prev = document.getElementById('nav_' + status + '_prev');
  if (current <= 0) {
    prev.disabled = true;
    prev.innerHTML = '&larr;';
  } else {
    prev.disabled = false;
    prev.innerHTML = '&larr; (' + lines[current - 1].id + ')';
  }

  var next = document.getElementById('nav_' + status + '_next');
  if (current >= lines.length - 1) {
    next.disabled = true;
    next.innerHTML = '&rarr;';
  } else {
    next.disabled = false;
    next.innerHTML = '&rarr; (' + lines[current + 1].id + ')';
  }
}

/** Callback function called when the user clicks on a navigator button
 *
 * param e - event
 * param status - kind of navigator (warning or error)
 * direction - 1 for next, -1 for previous
 */
function move_navigator(e, status, direction) {
  var navigator = window.navigators[status];
  var container = window.navigators.container;
  navigator.current += direction;
  var line = navigator.lines[navigator.current];

  // Move to the line position
  container.scrollTop = line.offsetTop - container.offsetTop;

  // Add a highlight effect
  line.classList.add('line_highlight');
  setTimeout(function() { line.classList.remove('line_highlight'); }, 100);

  update_navigator_buttons(status);
}

/** Reset the navigator buttons
 *
 * param e - event
 */
function reset_navigator(e) {
  window.navigators.error.current = -1;
  window.navigators.warning.current = -1;
  window.navigators.deadcode.current = -1;
  update_navigator_buttons('error');
  update_navigator_buttons('warning');
  update_navigator_buttons('deadcode');
}

/** Initialize the event for the call_contexts modal
 *
 * param e - event
 */
function init_call_contexts_modals(e) {
  var modal = document.getElementById('modal');
  modal.addEventListener('click', close_modal);
  var close = modal.getElementsByClassName('modal_close');
  for (var i = 0; i < close.length; i++) {
    close[i].addEventListener('click', close_modal);
  }
}

/** Callback function called when the user click on the call contexts toggle
 *
 * param e - event
 */
function toggle_call_contexts_modal(e) {
  var check = e.target.parentNode._check;
  var message = '';

  var function_name = window.functions[check.function_id];
  var call_context_ids = check.call_context_ids;
  for (var i = 0; i < call_context_ids.length; i++) {
    var call_context_id = call_context_ids[i];
    var call_context = window.call_contexts[call_context_id];

    if (i > 0) {
      message += '\n';
    }
    if (call_context === '') {
      message += 'Called from entry point \'' + function_name + '\'\n';
    } else {
      message += 'Called from:\n' + call_context + '\n';
    }
  }

  display_modal(message);
}

/** Display the modal window
 *
 * param content - the content of the modal window
 */
function display_modal(content) {
  var modal = document.getElementById('modal');
  var modal_content = document.getElementById('modal_call_context_content');
  modal_content.textContent = content;
  modal.classList.remove('hidden');
}

/** Close the modal */
function close_modal(e) {
  if (e.target.id !== 'modal' && e.target.className !== 'modal_close') {
    return;
  }
  var modal = document.getElementById('modal');
  modal.classList.add('hidden');
}

//** load events */
window.addEventListener('load', init_checks);
window.addEventListener('load', init_check_kinds_list);
window.addEventListener('load', init_status_checkbox);
window.addEventListener('load', init_navigators);
window.addEventListener('load', init_call_contexts_modals);
