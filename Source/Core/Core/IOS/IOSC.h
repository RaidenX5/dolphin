// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

// Implementation of an IOSC-like API, but much simpler since we only support actual keys.

#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "Common/CommonTypes.h"
#include "Common/Crypto/AES.h"

class PointerWrap;

namespace IOS
{
namespace HLE
{
enum ReturnCode : s32;

class IOSC final
{
public:
  using Handle = u32;

  enum class ConsoleType
  {
    Retail,
    RVT,
  };

  // We use the same default key handle IDs as the actual IOSC because there are ioctlvs
  // that accept arbitrary key handles from the PPC, so the IDs must match.
  // More information on default handles: https://wiibrew.org/wiki/IOS/Syscalls
  enum DefaultHandle : u32
  {
    // ECC-233 private signing key (per-console)
    HANDLE_CONSOLE_KEY = 0,
    // Console ID
    HANDLE_CONSOLE_ID = 1,
    // NAND FS AES-128 key
    HANDLE_FS_KEY = 2,
    // NAND FS HMAC
    HANDLE_FS_MAC = 3,
    // Common key
    HANDLE_COMMON_KEY = 4,
    // PRNG seed
    HANDLE_PRNG_KEY = 5,
    // SD AES-128 key
    HANDLE_SD_KEY = 6,
    // boot2 version (writable)
    HANDLE_BOOT2_VERSION = 7,
    // Unknown
    HANDLE_UNKNOWN_8 = 8,
    // Unknown
    HANDLE_UNKNOWN_9 = 9,
    // Filesystem version (writable)
    HANDLE_FS_VERSION = 10,
    // New common key (aka Korean common key)
    HANDLE_NEW_COMMON_KEY = 11,
  };

  enum ObjectType : u8
  {
    TYPE_SECRET_KEY = 0,
    TYPE_PUBLIC_KEY = 1,
    TYPE_DATA = 3,
  };

  enum ObjectSubType : u8
  {
    SUBTYPE_AES128 = 0,
    SUBTYPE_MAC = 1,
    SUBTYPE_ECC233 = 4,
    SUBTYPE_DATA = 5,
    SUBTYPE_VERSION = 6
  };

  IOSC(ConsoleType console_type = ConsoleType::Retail);
  ~IOSC();

  // Create an object for use with the other functions that operate on objects.
  ReturnCode CreateObject(Handle* handle, ObjectType type, ObjectSubType subtype, u32 pid);
  // Delete an object. Built-in objects cannot be deleted.
  ReturnCode DeleteObject(Handle handle, u32 pid);
  // Import a secret, encrypted key into dest_handle, which will be decrypted using decrypt_handle.
  ReturnCode ImportSecretKey(Handle dest_handle, Handle decrypt_handle, u8* iv,
                             const u8* encrypted_key, u32 pid);
  // Import a public key.
  ReturnCode ImportPublicKey(Handle dest_handle, const u8* public_key, u32 pid);
  // Compute an AES key from an ECDH shared secret.
  ReturnCode ComputeSharedKey(Handle dest_handle, Handle private_handle, Handle public_handle,
                              u32 pid);

  // AES encrypt/decrypt.
  ReturnCode Encrypt(Handle key_handle, u8* iv, const u8* input, size_t size, u8* output,
                     u32 pid) const;
  ReturnCode Decrypt(Handle key_handle, u8* iv, const u8* input, size_t size, u8* output,
                     u32 pid) const;

  // Ownership
  ReturnCode GetOwnership(Handle handle, u32* owner) const;
  ReturnCode SetOwnership(Handle handle, u32 owner, u32 pid);

  void DoState(PointerWrap& p);

private:
  struct KeyEntry
  {
    KeyEntry();
    KeyEntry(ObjectType type_, ObjectSubType subtype_, std::vector<u8>&& data_, u32 owner_mask_);
    void DoState(PointerWrap& p);

    bool in_use = false;
    ObjectType type;
    ObjectSubType subtype;
    std::vector<u8> data;
    u32 owner_mask = 0;
  };
  // The Wii's IOSC is limited to 32 entries, including 12 built-in entries.
  using KeyEntries = std::array<KeyEntry, 32>;

  void LoadDefaultEntries(ConsoleType console_type);
  KeyEntries::iterator FindFreeEntry();
  Handle GetHandleFromIterator(KeyEntries::iterator iterator) const;
  bool HasOwnership(Handle handle, u32 pid) const;
  bool IsDefaultHandle(Handle handle) const;
  ReturnCode DecryptEncrypt(Common::AES::Mode mode, Handle key_handle, u8* iv, const u8* input,
                            size_t size, u8* output, u32 pid) const;

  KeyEntries m_key_entries;
};
}  // namespace HLE
}  // namespace IOS
