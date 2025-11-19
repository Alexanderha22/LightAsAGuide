package com.team42.lightapp

import android.content.Context
import java.io.File


object SessionManager
{
    // Returns a list of the saved sessions' names
    // getSessionList() : List<String>

    // Returns the LightSession corresponding to the name
    // fun getSession(sessionName : String) : LightSession

    // Saves the session with the name, needs HardwareSystem for the sectionCount
    //fun saveSession(system: HardwareSystem, session: LightSession, sessionName : String)

    // Deletes the LightSession corresponding to the name
    // fun deleteSession(sessionName: String)


    // This function MUST be called before any files can be saved
    // SessionManager.getFolder(requireContext())
    var sessionFolder : File = File("")
    fun getFolder(context: Context)
    {
        sessionFolder = File(context.getFilesDir(), "Sessions")
        if (!sessionFolder.exists())
        {
            val success = sessionFolder.mkdirs()
        }
    }
}


fun SessionManager.getSessionList() : List<String>
{

    val sessionListFiles = sessionFolder.listFiles()
    val sessionList = mutableListOf<String>()

    if(sessionListFiles.isNullOrEmpty())
    {
        // Return empty
        return sessionList
    }

    for(file in sessionListFiles)
    {
        // Confirm filetype
        if(file.extension == "session")
        {
            sessionList.add(file.nameWithoutExtension)
        }
    }
    return sessionList
}

fun SessionManager.getSession(sessionName : String) : LightSession
{

    // Find specific file
    val file = File(sessionFolder, "$sessionName.session")

    // Create empty session to return
    val returnSession = mutableListOf<SessionBlock>()

    if (file.exists() && file.canRead())
    {
        parseFile(file, returnSession)
    }

    // Convert from mutable list to list
    return LightSession(sessionName, returnSession)
}

private fun parseFile(file : File, outSession : MutableList<SessionBlock>)
{
    val lines = file.readLines()

    try
    {
        // Throws error
        val sectionCount = lines[0].toInt()

        // Check for correct blocks. Each block has sectionCount+1 lines (Extra line for timestamp)
        val blockCount : Double = (lines.count() - 1).toDouble() / (sectionCount + 1)
        if(blockCount % 1 != 0.0)
        {
            // Invalid file
            return
        }

        // Get block data
        for(i in 0 until blockCount.toInt())
        {
            val index = 1 + i * (sectionCount + 1)

            // Timestamp, throws error
            val timeStamp = lines[index].toDouble()

            // Get all light data
            val lights = mutableListOf<LightSource>()
            for (j in 1 until 1+sectionCount)
            {
                // Brightness / Frequency, throws error
                val values = lines[index + j].split(",")
                val brightness = values[0].toDouble()
                val frequency = values[1].toDouble()

                lights.add(LightSource(brightness, frequency))
            }

            // Add the full block
            outSession.add(SessionBlock(lights, timeStamp))
        }
    }
    catch (_ : Throwable)
    {
        // Invalid conversion from string to double or int
        return
    }
}

fun SessionManager.saveSession(system: HardwareSystem, session: LightSession)
{

    // Find specific file, create if it does not exist
    val file = File(sessionFolder, "${session.name}.session")
    if(!file.exists())
    {
        file.createNewFile()
    }

    // Header info
    file.writeText(system.sectionCount.toString() + "\n")

    for(block in session.blocks)
    {
        // Timestamp
        file.appendText(block.timeStamp.toString() + "\n")

        // Light Info
        for(source in block.lights)
        {
            file.appendText("${source.brightness},${source.frequency}\n")
        }
    }
}

fun SessionManager.deleteSession(sessionName: String)
{
    // Find specific file, delete if it exists
    val file = File(sessionFolder, "$sessionName.session")
    if(file.exists())
    {
        file.delete()
    }
}