#include "conf.h"

#include <unistd.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <CUnit/Basic.h>

#include "desktop-entry.h"

int
setup_desktop_entry_suite (void) {
  FILE* fh = fopen ("tinto.desktop", "w");
  const char* entry_text = "[Desktop Entry]\n"
"Icon=firefox.png\n"
"Name=Firefox\n"
"Exec=firefox-bin\n";

  fwrite(entry_text, strlen (entry_text), 1, fh);

  fclose (fh);
  return 0;
}

int
teardown_desktop_entry_suite (void) {
  unlink ("tinto.desktop");
  return 0;
}

void
test_desktop_entry_creation (void) {
  desktop_entry_t* desk_entry =
    desktop_entry_create ("tinto.desktop");

  CU_ASSERT_PTR_NOT_NULL (desk_entry);
  CU_ASSERT_PTR_NOT_NULL (desk_entry->icon);
  CU_ASSERT_STRING_EQUAL(desk_entry->icon, "firefox.png");

  CU_ASSERT_PTR_NOT_NULL (desk_entry->exec);
  CU_ASSERT_STRING_EQUAL (desk_entry->exec, "firefox-bin");

  CU_ASSERT_PTR_NOT_NULL (desk_entry->name);
  CU_ASSERT_STRING_EQUAL (desk_entry->name, "Firefox");
  desktop_entry_destroy (desk_entry);

  desk_entry = desktop_entry_create ("acme-term.desktop");
  CU_ASSERT_PTR_NULL (desk_entry);
}

void
test_desktop_entry (void) {
  CU_pSuite de_suite = CU_add_suite("desktop-entry",
				    setup_desktop_entry_suite,
				    teardown_desktop_entry_suite);

  if (!CU_add_test(de_suite, "desktop-entry/creation",
		   test_desktop_entry_creation ))
    return ;
}
