/**
******************************************************************************
* @file    app_menu.c
* @author  MCD Application Team
* @brief   Application interface for menu
******************************************************************************
* @attention
*
* Copyright (c) 2020-2021 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "app_menu.h"
#include "app_conf.h"
#include <stdio.h>
// #include "stm32_lcd.h"
#include "stm32_seq.h"

/* External variables ------------------------------------------------------- */

/* Private defines ---------------------------------------------------------- */

/* Private variables -------------------------------------------------------- */
Menu_Page_t Menu_PagePool[MENU_MAX_PAGE];
tListNode Menu_PagePoolList;
tListNode Menu_PageActiveList;

Menu_Page_t *pCurrentPage;

extern uint8_t arrow_return_byteicon[];

/* Private functions prototypes-----------------------------------------------*/
static int32_t LCD_DrawBitmapArray(uint8_t xpos, uint8_t ypos, uint8_t xlen, uint8_t ylen, uint8_t *data);
static uint8_t Menu_ExecuteAction(Menu_Action_t Action);

/* Exported Functions Definition -------------------------------------------- */

/**
 * @brief Initialize the Menu module
 */
void Menu_Init(void)
{
  uint8_t i;

  LST_init_head(&Menu_PagePoolList);
  LST_init_head(&Menu_PageActiveList);

  for (i = 0; i < MENU_MAX_PAGE; i++)
  {
    Menu_PagePool[i].NumEntry = 0;
    Menu_PagePool[i].SelectedEntry = 0;
    LST_insert_tail(&Menu_PagePoolList, &Menu_PagePool[i].Node);
  }
}

/**
 * @brief Remove a Menu Page
 * @param MenuType: The type of the menu page
 * @retval A pointer to the menu page created, 0 if the pool list is empty
 */
Menu_Page_t* Menu_CreatePage(Menu_Type_t MenuType)
{
  Menu_Page_t *p_menu_page;
  if (LST_is_empty(&Menu_PagePoolList) == FALSE)
  {
    LST_remove_head(&Menu_PagePoolList, (tListNode **)&p_menu_page);
    p_menu_page->MenuType = MenuType;
    LST_insert_tail(&Menu_PageActiveList, (tListNode *)p_menu_page);

    if (MenuType == MENU_TYPE_LIST)
    {
      p_menu_page->SelectedEntry = 0;
      p_menu_page->CurrentStartId = 0;
    }

    return p_menu_page;
  }
  else
  {
    return 0;
  }
}

/**
 * @brief Remove a Menu Page
 * @param pMenuPage: A pointer to the menu page
 */
void Menu_RemovePage(Menu_Page_t* pMenuPage)
{
  if (LST_is_empty(&Menu_PageActiveList) == FALSE)
  {
    LST_remove_node((tListNode *) pMenuPage);
    LST_insert_tail(&Menu_PagePoolList, (tListNode *)pMenuPage);
  }
}

/**
 * @brief Add a list entry to a List menu page
 * @param pMenuPage: A pointer to the menu page
 * @param pText: The text of the entry
 * @param Action: The Action to assign to the list entry
 */
void Menu_AddListEntry(Menu_Page_t* pMenuPage, char *pText, Menu_Action_t Action)
{
  snprintf((char *)&pMenuPage->ListEntry[pMenuPage->NumEntry].Text, MENU_LIST_ENTRY_MAX_TEXT_LEN, "%s",pText);
  pMenuPage->ListEntry[pMenuPage->NumEntry].Action = Action;
  pMenuPage->NumEntry++;
}

/**
 * @brief Clear all entries in a List menu page
 * @param pMenuPage: A pointer to the menu page
 */
void Menu_ClearList(Menu_Page_t* pMenuPage)
{
  pMenuPage->NumEntry = 0;
  pMenuPage->SelectedEntry = 0;
}

/**
 * @brief Set the content of a Control Menu page
 * @param pMenuPage: A pointer to the menu page
 * @param pText: A pointer to a content text structure to display
 * @param pIcon: A pointer to an icon structure to discplay
 */
void Menu_SetControlContent(Menu_Page_t* pMenuPage, Menu_Content_Text_t *pText, Menu_Icon_t* pIcon)
{
  pMenuPage->pIcon = pIcon;
  pMenuPage->pText = pText;
}

/**
 * @brief Assign an action related to a direction on a menu page
 * @param pMenuPage: A pointer to the menu page
 * @param Direction: The direction to assign the action to
 * @param Action: The Action to assign
 */
void Menu_SetControlAction(Menu_Page_t* pMenuPage, Menu_Action_Direction_t Direction, Menu_Action_t Action)
{
  switch (Direction)
  {
    case MENU_DIRECTION_LEFT:
    {
      pMenuPage->ActionLeft = Action;
      break;
    }
    case MENU_DIRECTION_RIGHT:
    {
      pMenuPage->ActionRight = Action;
      break;
    }
    case MENU_DIRECTION_DOWN:
    {
      pMenuPage->ActionDown = Action;
      break;
    }
    case MENU_DIRECTION_UP:
    {
      pMenuPage->ActionUp = Action;
      break;
    }
  }
}

/**
 * @brief Get current active page
 * @return pMenuPage: A pointer to the current menu page
 */
Menu_Page_t* Menu_GetActivePage(void)
{
  return pCurrentPage;
}

/**
 * @brief Set the logo of a Logo Menu page
 * @param pMenuPage: A pointer to the menu page
 * @param pIcon: A pointer to an icon structure to discplay
 */
void Menu_SetLogo(Menu_Page_t* pMenuPage, Menu_Icon_t* pIcon)
{
  pMenuPage->pIcon = pIcon;
}

/**
 * @brief Set new page as active page
 * @param pMenuPage: A pointer to the menu page to set
 */
void Menu_SetActivePage(Menu_Page_t* pMenuPage)
{
  pCurrentPage = pMenuPage;
  Menu_Print();
}

/**
 * @brief Navigate in the menu, direction Left
 */
void Menu_Left(void)
{
  uint8_t ret;
  ret = Menu_ExecuteAction(pCurrentPage->ActionLeft);
  if (ret == 0 && pCurrentPage->pReturnPage != 0)
  {
    /* Return to previous menu page */
    pCurrentPage = (Menu_Page_t *) pCurrentPage->pReturnPage;
    Menu_Print();
  }
}

/**
 * @brief Navigate in the menu, direction Right
 */
void Menu_Right(void)
{
  switch (pCurrentPage->MenuType)
  {
    case MENU_TYPE_LIST:
    {
      Menu_ExecuteAction(pCurrentPage->ListEntry[pCurrentPage->SelectedEntry].Action);
      break;
    }
    case MENU_TYPE_CONTROL:
    {
      Menu_ExecuteAction(pCurrentPage->ActionRight);
      break;
    }
    default:
    break;
  }
}

/**
 * @brief Navigate in the menu, direction Up
 */
void Menu_Up(void)
{
  switch (pCurrentPage->MenuType)
  {
    case MENU_TYPE_LIST:
    {
      if (pCurrentPage->SelectedEntry == 0)
      {
        pCurrentPage->SelectedEntry = pCurrentPage->NumEntry - 1;
      }
      else
      {
        pCurrentPage->SelectedEntry = (pCurrentPage->SelectedEntry - 1) % pCurrentPage->NumEntry;
      }
      Menu_Print();
      break;
    }
    case MENU_TYPE_CONTROL:
    {
      Menu_ExecuteAction(pCurrentPage->ActionUp);
      break;
    }
    default:
    break;
  }
}

/**
 * @brief Navigate in the menu, direction Down
 */
void Menu_Down(void)
{
  switch (pCurrentPage->MenuType)
  {
    case MENU_TYPE_LIST:
    {
      pCurrentPage->SelectedEntry = (pCurrentPage->SelectedEntry + 1) % pCurrentPage->NumEntry;
      Menu_Print();
      break;
    }
    case MENU_TYPE_CONTROL:
    {
      Menu_ExecuteAction(pCurrentPage->ActionDown);
      break;
    }
    default:
    break;
  }
}

/**
 * @brief Request execution of the Print Task
 */
void Menu_Print(void)
{
#if (CFG_LCD_SUPPORTED == 1)
  UTIL_SEQ_SetTask( 1U << CFG_TASK_MENU_PRINT_ID, CFG_SEQ_PRIO_0);
#endif /* CFG_LCD_SUPPORTED */
}

/**
 * @brief Print the current menu on the screen
 */
void Menu_Print_Task(void)
{
  return;
}

/* Private Functions Definition --------------------------------------------- */

/**
 * @brief Draw an array of bits at the specified offsets starting from corner top left. Ensure xlen is multiple of 8
 * @param xpos: X coordinate to print at
 * @param ypos: Y coordinate to print at
 * @param xlen: Width of the bitmap array
 * @param ylen: Height of the bitmap array
 * @param data: Pointer to the bitmap array
 * @retval 0 if success, -1 if the coordinates are out of screen
 */
static int32_t LCD_DrawBitmapArray(uint8_t xpos, uint8_t ypos, uint8_t xlen, uint8_t ylen, uint8_t *data){

  int32_t i,j,k;
  uint8_t mask;
  uint8_t* pdata = data;

  if (((xpos+xlen) > 128) || ((ypos+ylen) > 64))
  {
    /*out of screen*/
    return -1;
  }

  return 0;
}

/**
 * @brief Execute Selected Action
 * @param Action: Action to execute
 */
static uint8_t Menu_ExecuteAction(Menu_Action_t Action)
{
  uint8_t ret = 0;

  if ((Action.ActionType & MENU_ACTION_CALLBACK) && Action.Callback != 0)
  {
    Action.Callback();
  }

  if ((Action.ActionType & MENU_ACTION_MENU_PAGE) && Action.pPage != 0)
  {
    ((Menu_Page_t *) Action.pPage)->pReturnPage = (struct Menu_Page_t *)pCurrentPage;
    pCurrentPage = (Menu_Page_t *) Action.pPage;
    if (pCurrentPage->MenuType == MENU_TYPE_LIST)
    {
      pCurrentPage->SelectedEntry = 0;
    }
    Menu_Print();
    ret = 1;
  }

  if ((Action.ActionType & MENU_ACTION_LIST_CALLBACK) && Action.ListCallback != 0)
  {
    Action.ListCallback(pCurrentPage->SelectedEntry);
  }
  return ret;
}
