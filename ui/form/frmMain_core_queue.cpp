#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"

#include "common/tableview_controller.h"
#include "ExcelReader.h"

#include <QList>
#include <QScriptEngine>

// 初始化队列
void frmMain::initQueue(){
    for (auto& item: g_config->getMeasuringStationConfig().scanEntries){
        MyQueue<PackageDto> queue;
        this->m_entryQueues.append(queue);
    }

    // 初始化队列
    auto rows = this->m_packBll->getCacheList();
    for (int i = rows.size()-1; i >= 0; i--){
        auto row = rows[i];
        auto dto = this->m_packBll->convertRow2Package(row);
        if (dto->status == PackageDto::StatusEnum::Status_Step4_Finish){
            this->m_waitingQueue.clear();
            for (int entryIndex = 0; i < this->m_entryQueues.size(); i++){
                this->m_entryQueues[i].clear();
            }
        }else if (dto->status == PackageDto::StatusEnum::Status_Step4_WaitingForSend){
            this->m_waitingQueue.enqueue(*dto);
        }else if (dto->status == PackageDto::StatusEnum::Status_Step3_GotScanTolerance
                  || dto->status == PackageDto::StatusEnum::Status_Step3_Waiting4ScanTolerance
                  || dto->status == PackageDto::StatusEnum::Status_Step2_GotMeasuringHeight
                  || dto->status == PackageDto::StatusEnum::Status_Step2_Waiting4MeasuringHeight
                  || dto->status == PackageDto::StatusEnum::Status_Step2_Waiting4SendPackNo){
            auto ip = row->data(PackBLL::PackColEnum::OriginIp).toString();
            auto entryIndex = this->getScanEntryIndex(ip);

            this->m_entryQueues[entryIndex].enqueue(*dto);
        }
    }

    // 状态栏
    QIcon nullIcon;
    this->m_customStatusBar->
            addIcon(nullIcon, this->StatusBar_IconName_Queue, "", nullptr);

}

// 处理队列中的任务
void frmMain::processTasks()
{
    QStringList messages;

    // 查看入口队列
    int index = 1;
    for(auto& q : this->m_entryQueues){ //TODO 启用多个线程来处理，最后等待两个线程都结束，才处理下一个
        if (!q.isEmpty())
        {
            auto head = q.peek();
            // 处于等待发送剁标识到拼板口时，需要检查当前拼板口是否允许写入数据
            if (head->status == PackageDto::StatusEnum::Status_Step2_Waiting4SendPackNo){
                // 是否拼板口允许写入
                int scanEntryIndex = this->getScanEntryIndex(head->originIp);
                if (!this->isAllowWrite2PanelDockingStation(scanEntryIndex)){
                    //TODO 在状态栏有体现
                    break;
                }

                // 写入数据
                auto targeStatus = PackageDto::StatusEnum::Status_Step2_SentPackNo;
                this->runFlow(*head, &targeStatus);

            }
            // 当状态处于“获取测高数据”之后时
            else if (head->status >= PackageDto::StatusEnum::Status_Step2_GotMeasuringHeight){
                auto item = q.dequeue(); // 完成了上面操作后再出队，确保队列中的元素都处理完成
                if (item){
                    if (item->id != head->id){
                        throw std::runtime_error("出队数据 与 队列头部数据不匹配！");
                    }
                    this->m_waitingQueue.enqueue(*item); // 加入到发送队列
                }

            }
        }

        messages.append("entry"
                        +QString::number(index)+" "
                        +QString::number(q.length()));
        index++;
    }

    // 加工队列
    if (!this->m_waitingQueue.isEmpty()){
        auto item = this->m_waitingQueue.peek(); // 读取非出队
        if (!item){
            throw std::runtime_error("从待加工队列中读取成员错误");
        }

        // 流转
        auto status = PackageDto::StatusEnum::Status_Step4_Sent;
        this->runFlow(*item, &status);
        this->m_waitingQueue.dequeue(); // 完成了上面操作后再出队，确保队列中的元素都处理完成        
    }
    messages.append("work "+QString::number(this->m_waitingQueue.length()));

    this->m_customStatusBar->
            setTooltip(this->StatusBar_IconName_Queue, messages.join("\n"));
}
