#include <stdio.h>
#include <string.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>
#include <emscripten/val.h>

#include "json.hpp"

/*
Fetch to download
Val to access dom
HTML5 for event handling
*/

using namespace nlohmann;
using namespace emscripten;

std::string getInput (std::string fieldName) 
{
  val document = val::global("document");
  val value = document.call<val>("getElementById", fieldName)["value"];
    return value.as<std::string>();
}

void setInput (std::string fieldname, std::string value )
{
  val document = val::global("document");
  val elem = document.call<val>("getElementById", std::string(fieldname));                    
  elem.set("value", value);
}

void DisplayFetchedData(emscripten_fetch_t *fetch)
{
  // Null terminates data, dodgy, may overwrite something in memory
  ((char*)fetch->data)[fetch->numBytes]='\0';
    
  printf ("%s\n", fetch->data);
  auto j4 = json::parse(fetch->data);
  setInput("id", j4["id"]);
  setInput("name", j4["name"]);
  setInput("address", j4["address"]);
}

void downloadSucceeded(emscripten_fetch_t *fetch) {
  printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);    
  DisplayFetchedData(fetch);  
  emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void downloadFailed(emscripten_fetch_t *fetch) {
  printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
  emscripten_fetch_close(fetch); // Also free data on failure.
}

int on_load_click(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
  char* jsonUrl;
  asprintf(&jsonUrl, "/HTML5Api/data%s.json", getInput("id").c_str());

  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_REPLACE;
  attr.onsuccess = downloadSucceeded;
  attr.onerror = downloadFailed;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, jsonUrl);   

  return 0;
}

int on_clear_click(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
  setInput("id", "");
  setInput("name", "");
  setInput("address", "");
  return 0;
}

int main()
{
  emscripten_set_click_callback("btLoad", (void*)0, 0, on_load_click);
  emscripten_set_click_callback("btClear", (void*)0, 0, on_clear_click);
  
  // To avoid exit after leaving main(), keeping events alive.
  EM_ASM(Module['noExitRuntime'] = true);
  
  return 0;
}