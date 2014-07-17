#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cardTableExtension.hpp	1.11 03/01/23 12:01:52 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class MutableSpace;
class ObjectStartArray;
class PSPromotionManager;
class GCTaskQueue;

class CardTableExtension : public CardTableModRefBS {
 protected:

  static void verify_all_young_refs_precise_helper(MemRegion mr);

 public:
  enum ExtendedCardValue {
    youngergen_card   = CardTableModRefBS::CT_MR_BS_last_reserved + 1,
    verify_card       = CardTableModRefBS::CT_MR_BS_last_reserved + 5
  };

  CardTableExtension(MemRegion whole_heap, int max_covered_regions) :
    CardTableModRefBS(whole_heap, max_covered_regions) { }

  // Too risky for the 4/10/02 putback
  // BarrierSet::Name kind() { return BarrierSet::CardTableExtension; }

  // Scavenge support
  void scavenge_contents(ObjectStartArray* start_array,
                         MutableSpace* sp,
                         HeapWord* space_top,
                         PSPromotionManager* pm);
    
  void scavenge_contents_parallel(ObjectStartArray* start_array,
                                  MutableSpace* sp,
                                  HeapWord* space_top,
                                  PSPromotionManager* pm,
                                  uint stripe_number);

  // Verification
  static void verify_all_young_refs_imprecise();
  static void verify_all_young_refs_precise();

  bool addr_is_marked_imprecise(void *addr);
  bool addr_is_marked_precise(void *addr);

  void set_card_newgen(void* addr)   { jbyte* p = byte_for(addr); *p = verify_card; }

  // Testers for entries
  static bool card_is_dirty(int value)      { return value == dirty_card; }
  static bool card_is_newgen(int value)     { return value == youngergen_card; }
  static bool card_is_clean(int value)      { return value == clean_card; }
  static bool card_is_verify(int value)     { return value == verify_card; }

  // Card marking
  void inline_write_ref_field_gc(oop* field, oop new_val) {
    jbyte* byte = byte_for(field);
    *byte = youngergen_card;
  }

#ifdef ASSERT

  bool is_valid_card_address(jbyte* addr) {
    return (addr >= _byte_map) && (addr < _byte_map + _byte_map_size);
  }
  
#endif // ASSERT
};

