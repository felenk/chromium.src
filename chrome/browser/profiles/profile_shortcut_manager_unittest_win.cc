// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/file_util.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/test/scoped_path_override.h"
#include "base/string16.h"
#include "base/test/test_shortcut_win.h"
#include "base/utf_string_conversions.h"
#include "base/win/shortcut.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/profiles/profile_shortcut_manager.h"
#include "chrome/installer/util/browser_distribution.h"
#include "chrome/installer/util/shell_util.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/testing_pref_service.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/base/testing_profile_manager.h"
#include "content/public/test/test_browser_thread.h"
#include "grit/theme_resources.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/resource/resource_bundle.h"

using content::BrowserThread;

namespace {

// Sets |path| to the path to the shortcut for this profile.
// Automatically fails the test on failure to do so.
void GetShortcutPathForProfile(BrowserDistribution* dist,
                               const string16& profile_name,
                               FilePath* path) {
  EXPECT_TRUE(ShellUtil::GetShortcutPath(ShellUtil::SHORTCUT_DESKTOP, dist,
                                         ShellUtil::CURRENT_USER, path));
  string16 shortcut_name(
      ProfileShortcutManager::GetShortcutNameForProfile(profile_name));
  *path = path->Append(shortcut_name);
}

// Returns true if the shortcut for this profile exists.
bool ProfileShortcutExists(BrowserDistribution* dist,
                           const string16& profile_name) {
  FilePath shortcut_path;
  GetShortcutPathForProfile(dist, profile_name, &shortcut_path);
  return file_util::PathExists(shortcut_path);
}

// Calls base::win::ValidateShortcut() with expected properties for
// |profile_name|'s shortcut.
void ValidateProfileShortcut(BrowserDistribution* dist,
                             const string16& profile_name) {
  EXPECT_TRUE(ProfileShortcutExists(dist, profile_name));

  FilePath exe_path;
  EXPECT_TRUE(PathService::Get(base::FILE_EXE, &exe_path));

  FilePath shortcut_path;
  GetShortcutPathForProfile(dist, profile_name, &shortcut_path);

  // TODO(hallielaine): With this new struct method for VerifyShortcut you can
  // now test more properties like: arguments, icon, icon_index, and app_id.
  base::win::ShortcutProperties expected_properties;
  expected_properties.set_target(exe_path);
  expected_properties.set_description(dist->GetAppDescription());
  expected_properties.set_dual_mode(false);
  base::win::ValidateShortcut(shortcut_path, expected_properties);
}

}  // namespace

class ProfileShortcutManagerTest : public testing::Test {
 protected:
  ProfileShortcutManagerTest()
      : ui_thread_(BrowserThread::UI, &message_loop_),
        file_thread_(BrowserThread::FILE, &message_loop_),
        fake_user_desktop_(base::DIR_USER_DESKTOP) {
  }

  virtual void SetUp() OVERRIDE {
    dist_ = BrowserDistribution::GetDistribution();
    ASSERT_TRUE(dist_ != NULL);

    TestingBrowserProcess* browser_process =
        static_cast<TestingBrowserProcess*>(g_browser_process);
    profile_manager_.reset(new TestingProfileManager(browser_process));
    ASSERT_TRUE(profile_manager_->SetUp());

    dest_path_ = profile_manager_->profile_info_cache()->GetUserDataDir();
    dest_path_ = dest_path_.Append(FILE_PATH_LITERAL("My profile"));
    file_util::CreateDirectoryW(dest_path_);
    profile_name_ = ASCIIToUTF16("My profile");

    second_dest_path_ =
        profile_manager_->profile_info_cache()->GetUserDataDir();
    second_dest_path_ =
        second_dest_path_.Append(FILE_PATH_LITERAL("My profile 2"));
    file_util::CreateDirectoryW(second_dest_path_);
    second_profile_name_ = ASCIIToUTF16("My profile 2");
  }

  virtual void TearDown() OVERRIDE {
    message_loop_.RunAllPending();

    int num_profiles =
        profile_manager_->profile_info_cache()->GetNumberOfProfiles();

    // Remove all shortcuts except the last (since it will no longer have
    // an appended name).
    for (int i = 0; i < num_profiles; ++i) {
      const FilePath profile_path =
          profile_manager_->profile_info_cache()->GetPathOfProfileAtIndex(0);
      string16 profile_name;
      if (i != num_profiles - 1) {
        profile_name =
            profile_manager_->profile_info_cache()->GetNameOfProfileAtIndex(0);
      }
      profile_manager_->profile_info_cache()->DeleteProfileFromCache(
          profile_path);
      MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
      MessageLoop::current()->Run();
      ASSERT_FALSE(ProfileShortcutExists(dist_, profile_name));
      ASSERT_FALSE(file_util::PathExists(profile_path.Append(
          FILE_PATH_LITERAL("Google Profile.ico"))));
    }
  }

  void SetupDefaultProfileShortcut() {
    ASSERT_FALSE(ProfileShortcutExists(dist_, profile_name_));
    // A non-badged shortcut for chrome is automatically created with the
    // first profile (for the case when the user deletes their only
    // profile).
    profile_manager_->profile_info_cache()->AddProfileToCache(
        dest_path_, profile_name_, string16(), 0);
    MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
    MessageLoop::current()->Run();
    // We now have 1 profile, so we expect a new shortcut with no profile
    // information.
    ValidateProfileShortcut(dist_, string16());
  }

  void SetupAndCreateTwoShortcuts() {
    ASSERT_EQ(0, profile_manager_->profile_info_cache()->GetNumberOfProfiles());
    ASSERT_FALSE(ProfileShortcutExists(dist_, profile_name_));
    ASSERT_FALSE(ProfileShortcutExists(dist_, second_profile_name_));

    profile_manager_->profile_info_cache()->AddProfileToCache(
        dest_path_, profile_name_, string16(), 0);
    profile_manager_->profile_info_cache()->AddProfileToCache(
        second_dest_path_, second_profile_name_, string16(), 0);
    profile_manager_->profile_manager()->profile_shortcut_manager()->
        CreateProfileShortcut(second_dest_path_);
    MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
    MessageLoop::current()->Run();
    ValidateProfileShortcut(dist_, profile_name_);
    ValidateProfileShortcut(dist_, second_profile_name_);
  }

  BrowserDistribution* dist_;
  MessageLoopForUI message_loop_;
  content::TestBrowserThread ui_thread_;
  content::TestBrowserThread file_thread_;
  scoped_ptr<TestingProfileManager> profile_manager_;
  base::ScopedPathOverride fake_user_desktop_;
  FilePath dest_path_;
  string16 profile_name_;
  FilePath second_dest_path_;
  string16 second_profile_name_;
};

TEST_F(ProfileShortcutManagerTest, DesktopShortcutsCreate) {
  if (!profile_manager_->profile_manager()->profile_shortcut_manager())
    return;
  ProfileShortcutManagerTest::SetupDefaultProfileShortcut();

  profile_manager_->profile_info_cache()->AddProfileToCache(
      second_dest_path_, second_profile_name_, string16(), 0);
  profile_manager_->profile_manager()->profile_shortcut_manager()->
      CreateProfileShortcut(second_dest_path_);
  MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
  MessageLoop::current()->Run();

  // We now have 2 profiles, so we expect a new shortcut with profile
  // information for this 2nd profile.
  ValidateProfileShortcut(dist_, second_profile_name_);
  ASSERT_TRUE(file_util::PathExists(second_dest_path_.Append(
      FILE_PATH_LITERAL("Google Profile.ico"))));
}

TEST_F(ProfileShortcutManagerTest, DesktopShortcutsUpdate) {
  if (!profile_manager_->profile_manager()->profile_shortcut_manager())
    return;
  ProfileShortcutManagerTest::SetupDefaultProfileShortcut();

  EXPECT_FALSE(ProfileShortcutExists(dist_, second_profile_name_));

  profile_manager_->profile_info_cache()->AddProfileToCache(
      second_dest_path_, second_profile_name_, string16(), 0);
  profile_manager_->profile_manager()->profile_shortcut_manager()->
      CreateProfileShortcut(second_dest_path_);
  MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
  MessageLoop::current()->Run();
  ValidateProfileShortcut(dist_, second_profile_name_);

  // Cause an update in ProfileShortcutManager by modifying the profile info
  // cache.
  string16 new_profile_name = ASCIIToUTF16("New Profile Name");
  profile_manager_->profile_info_cache()->SetNameOfProfileAtIndex(
      profile_manager_->profile_info_cache()->GetIndexOfProfileWithPath(
          second_dest_path_),
      new_profile_name);
  MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
  MessageLoop::current()->Run();
  EXPECT_FALSE(ProfileShortcutExists(dist_, second_profile_name_));
  ValidateProfileShortcut(dist_, new_profile_name);
}

TEST_F(ProfileShortcutManagerTest, DesktopShortcutsDeleteSecondToLast) {
  if (!profile_manager_->profile_manager()->profile_shortcut_manager())
    return;
  ProfileShortcutManagerTest::SetupAndCreateTwoShortcuts();

  // Delete one shortcut
  profile_manager_->profile_info_cache()->DeleteProfileFromCache(
      second_dest_path_);
  MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
  MessageLoop::current()->Run();
  EXPECT_FALSE(ProfileShortcutExists(dist_, second_profile_name_));

  // Verify that the profile name has been removed from the remaining shortcut
  ValidateProfileShortcut(dist_, string16());
  // Verify that an additional shortcut, with the default profile's name does
  // not exist
  EXPECT_FALSE(ProfileShortcutExists(dist_, profile_name_));
}

TEST_F(ProfileShortcutManagerTest, DesktopShortcutsCreateSecond) {
  if (!profile_manager_->profile_manager()->profile_shortcut_manager())
    return;
  ProfileShortcutManagerTest::SetupAndCreateTwoShortcuts();

  // Delete one shortcut
  profile_manager_->profile_info_cache()->DeleteProfileFromCache(
      second_dest_path_);
  MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
  MessageLoop::current()->Run();

  // Verify that a default shortcut exists (no profile name/avatar)
  ValidateProfileShortcut(dist_, string16());
  // Verify that an additional shortcut, with the default profile's name does
  // not exist
  EXPECT_FALSE(ProfileShortcutExists(dist_, profile_name_));

  // Create a second profile and shortcut
  profile_manager_->profile_info_cache()->AddProfileToCache(
       second_dest_path_, second_profile_name_, string16(), 0);
  profile_manager_->profile_manager()->profile_shortcut_manager()->
      CreateProfileShortcut(second_dest_path_);
  MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
  MessageLoop::current()->Run();
  ValidateProfileShortcut(dist_, second_profile_name_);

  // Verify that the original shortcut received the profile's name
  ValidateProfileShortcut(dist_, profile_name_);
  // Verify that a default shortcut no longer exists
  EXPECT_FALSE(ProfileShortcutExists(dist_, string16()));
}
