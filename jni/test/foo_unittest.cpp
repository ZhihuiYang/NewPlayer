#include <gtest/gtest.h>
#include "linkedList.h"

//using testing::Environment;
//
//int main(int argc, char *argv[])
//{
//    testing::AddGlobalTestEnvironment(NULL);
//    testing::InitGoogleTest(&argc, argv);
//    int ret = RUN_ALL_TESTS();
//    return ret;
//}

TEST(linkedListTest, linkedListGetSizeTest)
{   
    pLinkedList pLL = createLinkedList();
    pLL->linkedListInsertDataAtFirst(pLL,(void *)"a");
    EXPECT_EQ(1, pLL->linkedListGetSize(pLL));
}

