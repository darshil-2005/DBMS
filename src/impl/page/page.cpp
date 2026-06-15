
#include "../../include/page/page.h"


Bool InternalPage::CheckSlotAvailable(Byte* page, uint16_t key_size) {
  InternalPageHeader* page_header = reinterpret_cast<InternalPageHeader*>(page);
  if (page_header->num_keys < NUM_KEY_SLOTS) {
    return 1;
  };
  return 0;
};


Key* InternalPage::GetKeysStartPointer(Byte* page){
  return reinterpret_cast<Key*>(page + INTERNAL_PAGE_HEADER_SIZE);
};

PageID* InternalPage::GetChildrenStartPointer(Byte* page) {
  return InternalPage::GetKeysStartPointer(page) + NUM_KEY_SLOTS;
};

PageID InternalPage::GetValueAtIndex(Byte* page, OffsetIndex index) {
  PageID* value_start = InternalPage::GetChildrenStartPointer(page);
  PageID res = *(value_start + index);
  return res;
};

PageID InternalPage::GetChildPageID(Byte* page, uint16_t key) {
  InternalPageHeader* page_header = reinterpret_cast<InternalPageHeader*>(page);
  Key* key_start = GetKeysStartPointer(page);
  Key* key_end = key_start + page_header->num_keys;

  Key* it = std::upper_bound(key_start, key_end, key);
  OffsetIndex index_child_ptr = it - key_start;

  return GetValueAtIndex(page, index_child_ptr);
};

// ALERT: This will overwrite the child pointers if you enter more keys than capacity.
Bool InternalPage::InsertKeyValue(Byte* page, Key boundary_key, PageID new_pid) {
    
  InternalPageHeader* page_header = reinterpret_cast<InternalPageHeader*>(page);
  Key* key_start = GetKeysStartPointer(page);
  Key* key_end = key_start + page_header->num_keys;
  Key* it = std::upper_bound(key_start, key_end, boundary_key);
  OffsetIndex index_child_ptr = it - key_start;

  memmove(it+1, it, (key_end - it) * KEY_SIZE);

  *it = boundary_key;
  OffsetIndex insertion_index = index_child_ptr + 1;

  PageID* value_start = GetChildrenStartPointer(page);
  PageID* value_end = value_start + page_header->num_keys + 1;
  PageID* insertion_it = value_start + insertion_index;

  if (insertion_it != value_end) {
    memmove(insertion_it+1, insertion_it, (value_end - insertion_it) * sizeof(PageID));
  };
  page_header->num_keys++;

  *insertion_it = new_pid;
  return 1;
};

uint16_t InternalPage::HandleSplit(Byte* old_page, Byte* new_page, Key key_to_insert, PageID page_id_to_insert) {

  Key temp_keys[NUM_KEY_SLOTS + 1];
  PageID temp_ptrs[NUM_CHILD_PAGEID_SLOTS + 1];
  InternalPageHeader* page_header = reinterpret_cast<InternalPageHeader*>(old_page);
  Key* key_start = InternalPage::GetKeysStartPointer(old_page);
  Key* ptr_start = InternalPage::GetChildrenStartPointer(old_page);

  memcpy(temp_keys, key_start, page_header->num_keys * KEY_SIZE);
  memcpy(temp_ptrs, ptr_start, (page_header->num_keys + 1) * CHILD_PTR_SIZE);

  Key* it = std::upper_bound(temp_keys, temp_keys + page_header->num_keys, key_to_insert);
  
  if (it != temp_keys + page_header->num_keys) {
    memmove(it + 1, it, (temp_keys + page_header->num_keys - it) * KEY_SIZE);
  };

  *it = key_to_insert;
  OffsetIndex index_child_ptr = it - temp_keys;
  OffsetIndex insertion_index = index_child_ptr + 1;

  PageID* insertion_it = temp_ptrs + insertion_index;

  if (insertion_it != temp_ptrs + page_header->num_keys + 1) {
    memmove(insertion_it+1, insertion_it, ((temp_ptrs + page_header->num_keys + 1) - insertion_it) * sizeof(PageID));
  };

  *insertion_it = page_id_to_insert;

  uint16_t new_keys_length = page_header->num_keys + 1; 

  Key* boundary_key = temp_keys + (new_keys_length / 2);
  // correction
  InternalPage::MakePage(old_page, temp_keys, temp_ptrs, (new_keys_length / 2), page_header->page_id);
  InternalPage::MakePage(new_page, boundary_key + 1, temp_ptrs + (new_keys_length / 2) + 1, ((new_keys_length - 1) / 2), page_id_to_insert);

  return *boundary_key;  
};

/*
uint16_t InternalPage::HandleSplit(Byte* old_page, Byte* new_page, Key key_to_insert, PageID page_id_to_insert) {

  Key temp_keys[NUM_KEY_SLOTS + 1];
  PageID temp_ptrs[NUM_CHILD_PAGEID_SLOTS+1];
  InternalPageHeader* page_header = reinterpret_cast<InternalPageHeader*>(old_page);
  Key* key_start = GetKeysStartPointer(old_page);
  Key* ptr_start = GetChildrenStartPointer(old_page);

  memcpy(temp_keys, key_start, page_header->num_keys * KEY_SIZE);
  memcpy(temp_ptrs, ptr_start, (page_header->num_keys + 1) * CHILD_PTR_SIZE);

  Key* it = std::upper_bound(temp_keys, temp_keys + page_header->num_keys, key_to_insert);
  
  if (it != temp_keys + page_header->num_keys) {
    memmove(it + 1, it, (temp_keys + page_header->num_keys - it) * KEY_SIZE);
  };

  *it = key_to_insert;
  OffsetIndex index_child_ptr = it - temp_keys;
  OffsetIndex insertion_index = index_child_ptr + 1;

  PageID* insertion_it = temp_ptrs + insertion_index;

  if (insertion_it != temp_ptrs + page_header->num_keys + 1) {
    memmove(insertion_it+1, insertion_it, ((temp_ptrs + page_header->num_keys + 1) - insertion_it) * sizeof(PageID));
  };

  *insertion_it = page_id_to_insert;

  uint16_t new_keys_length = page_header->num_keys + 1; 

  Key* boundary_key = temp_keys + (new_keys_length / 2);
  // correction
  MakePage(old_page, temp_keys, temp_ptrs, (new_keys_length / 2), page_header->page_id, page_header->parent_pid);
  MakePage(new_page, boundary_key + 1, temp_ptrs + (new_keys_length / 2) + 1, ((new_keys_length - 1) / 2), page_id_to_insert, page_header->parent_pid);

  return *boundary_key;  
};
*/

// ALERT: This might overwrite the keys into the space for child ptr and write child_ptr over the boundary of the page.
Bool InternalPage::MakePage(Byte* page, Key* keys_ptr, PageID* children_ptr, uint16_t keys_to_take, PageID pid) {
  
  Byte* curr = page;
  *curr = static_cast<Byte>(PageType::InternalPage);

  curr = curr + sizeof(PageType::InternalPage);
  // correction
  memcpy(curr, &pid, sizeof(pid));

  curr = curr + sizeof(pid);
  memcpy(curr, &keys_to_take, sizeof(keys_to_take)); 

  curr = curr + sizeof(keys_to_take);
  memcpy(curr, keys_ptr, (keys_to_take * sizeof(*keys_ptr)));

  curr = curr + sizeof(*keys_ptr) * NUM_KEY_SLOTS;
  memcpy(curr, children_ptr, sizeof(*children_ptr) * (keys_to_take + 1));
  
  return 1;
};

/*

// need to handle both leaf and overflow page
uint16_t LeafPage::CheckAvailableSpace(Byte* page) {
  
  LeafPageHeader* page_header = reinterpret_cast<LeafPageHeader*>(page);

  Byte* free_space_start = page;
  Byte* free_space_end = page + page_header->free_space_end_offset;
  free_space_start = free_space_start + LEAF_PAGE_HEADER_SIZE;
  free_space_start = free_space_start + (page_header->slot_array_size * SLOT_SIZE);

  uint16_t free_bytes = free_space_end - free_space_start + 1;
  return free_bytes;
};

SlotArrayElement* upper_bound(const SlotArrayElement* start, const SlotArrayElement* end, Byte* page, Key x) {

  int16_t n = end - start;
  int16_t l = 0;
  int16_t r = n - 1;
  int16_t ans = n;

  while (l <= r) {
    uint16_t mid = l + (r - l) / 2;
    SlotArrayElement* element = start + mid;
    Byte* tuple = page + element->offset;
    Key* key = reinterpret_cast<Key*>(tuple + TUPLE_HEADER_SIZE); 
    if (*key <= x) {
      l = mid + 1;
    } else {
      ans = mid;
      r = mid - 1;
    };
  };

  return start + ans;
};


// The caller assumes that this function can store atleast 1 tuple and creates chain of tuples if required.
// All of the chaining into overflow pages is abstracted away by this function.
RecordID LeafPage::InsertTuple(Byte* page, const Byte *buffer, BufferSize buffer_size, Key key) {

  PageHeader* general_page_header = reinterpret_cast<PageHeader*>(page);

  uint16_t page_data_limit = (general_page_header->page_type == PageType::LeafPage) 
    ? LEAF_TUPLE_DATA_SIZE_MAX 
    : OVERFLOW_TUPLE_DATA_SIZE_MAX;  

  uint16_t page_header_size = (general_page_header->page_type == PageType::LeafPage) 
    ? LEAF_PAGE_HEADER_SIZE 
    : OVERFLOW_PAGE_HEADER_SIZE;

  uint16_t slot_array_size = (general_page_header->page_type == PageType::LeafPage) 
    ? reinterpret_cast<LeafPageHeader*>(page)->slot_array_size 
    : reinterpret_cast<OverflowPageHeader*>(page)->slot_array_size;

  Offset free_space_end_offset = (general_page_header->page_type == PageType::LeafPage)
    ? reinterpret_cast<LeafPageHeader*>(page)->free_space_end_offset
    : reinterpret_cast<OverflowPageHeader*>(page)->free_space_end_offset;

  Bool* overflow = (general_page_header->page_type == PageType::LeafPage)
    ? reinterpret_cast<LeafPageHeader*>(page)->overflow
    : reinterpret_cast<OverflowPageHeader*>(page)->overflow;

  PageID* next_page_id = (general_page_header->page_type == PageType::LeafPage)
    ? reinterpret_cast<LeafPageHeader*>(page)->next_page_id
    : reinterpret_cast<OverflowPageHeader*>(page)->next_page_id;


  uint16_t available_space = CheckAvailableSpace(page);

    if (buffer_size <= page_data_limit) {
      // the tuple is already made we just have to place the data and the overflow info is inside the slot elements rather than the tuples.
      if (buffer_size + SLOT_SIZE <= available_space) {
        // store all data here in this page.
        // set overflow false in the slot array element

        uint16_t data_size = buffer_size;
              
        SlotArrayElement* slot_array_start = page + page_header_size;
        SlotArrayElement* slot_array_end = page + (SLOT_SIZE * slot_array_size);

        SlotArrayElement* it = upper_bound(slot_array_start, slot_array_end, page, key);
        if (it != slot_array_end) memmove(it+SLOT_SIZE, it, (slot_array_end - it) * SLOT_SIZE);

        it->offset = free_space_end_offset - data_size + 1;
        it->length = data_size;
        it->overflow = 0;
        it->next_record = { .pid = 0, .slot_index = 0 };

        memcpy(page + it->offset, buffer, data_size); 

        return { .pid = general_page_header->page_id, .slot_index = (OffsetIndex)(it - slot_array_start) };

      } else {
        // store data of size available_space - SLOT_SIZE here
        // set overflow flag to true and recurse
        // set the next_record to the returned recordID
        uint16_t data_size = available_space - SLOT_SIZE;

        SlotArrayElement* slot_array_start = page + page_header_size;
        SlotArrayElement* slot_array_end = page + (SLOT_SIZE * slot_array_size);

        SlotArrayElement* it = upper_bound(slot_array_start, slot_array_end, page, key);
        if (it != slot_array_end) memmove(it+SLOT_SIZE, it, (slot_array_end - it) * SLOT_SIZE);

        it->offset = free_space_end_offset - data_size + 1;
        it->length = data_size;

        memcpy(page + it->offset, buffer, data_size); 

        if (overflow == 0) {
          NewPage new_page = B

          
        };



        // RecordID LeafPage::InsertTuple(Byte* page, const Byte *buffer, BufferSize buffer_size, Key key) {
        RecordID overflow_record = InsertTuple(



      };
    } else {
      if (page_data_limit + SLOT_SIZE <= available_space) {
        // store page_data_limit - SLOT_SIZE sized data here
        // set overflow flag to true and recurse
        // set the next_record to the returned recordID
      } else {
        // store available_space sized data here
        // set overflow flag to true and recurse
        // set the next_record to the returned recordID
      }
    };


  SlotArrayElement* slot_array_start = page + LEAF_PAGE_HEADER_SIZE;
  SlotArrayElement* slot_array_end = page + (SLOT_SIZE * page_header->slot_array_size);

  SlotArrayElement* it = upper_bound(slot_array_start, slot_array_end, page, key);
  if (it != slot_array_end) memmove(it+SLOT_SIZE, it, (slot_array_end - it) * SLOT_SIZE);

};

*/

