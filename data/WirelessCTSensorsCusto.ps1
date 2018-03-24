#Script to prepare Web files
# - Integrate customization of the application
# - GZip of resultant web files
# - and finally convert compressed web files to C++ header in PROGMEM

#List here web files specific to this project/application
$specificFiles="",""

#list here files that need to be merged with Common templates
$applicationName="WirelessCTSensors"
$shortApplicationName="WCTSensors"
$templatesWithCustoFiles=@{
    #---Status.html---
    "status.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
        ;
        HTMLContent=@'
        <h2 class="content-subhead">CTSensors <span id="l3"><h6 style="display:inline"><b> Loading...</b></h6></span></h2>
        Current I 1 : <span id="ci1"></span> (average : <span id="cai1"></span>)<br>
        Current I 2 : <span id="ci2"></span> (average : <span id="cai2"></span>)<br>
        Current I 3 : <span id="ci3"></span> (average : <span id="cai3"></span>)<br>
        <br>
        Counter 1 : <span id="c1"></span><br>
        Counter 2 : <span id="c2"></span><br>
        Counter 2 : <span id="c3"></span><br>
        <dl>
            <dt>Jeedom Upload</dt>
            <dd>Last Request Clamp 1 : <span id="lr1"></span></dd>
            <dd>Last Request Result Clamp 1 : <span id="lrr1"></span></dd>
            <dd>Last Request Clamp 2 : <span id="lr2"></span></dd>
            <dd>Last Request Result Clamp 2 : <span id="lrr2"></span></dd>
            <dd>Last Request Clamp 3 : <span id="lr3"></span></dd>
            <dd>Last Request Result Clamp 3 : <span id="lrr3"></span></dd>
        </dl>
'@
        ;
        HTMLScriptInReady=@'
        $.getJSON("/gs1", function(GS1){

            $.each(GS1,function(k,v){
                $('#'+k).html(v);
            })
            $("#l3").fadeOut();
        })
        .fail(function(){
            $("#l3").html('<h4 style="display:inline;color:red;"><b> Failed</b></h4>');
        });
'@
    }
    ;
    #---config.html---
    "config.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
        ;
        HTMLContent=@'
        <h2 class="content-subhead">CTSensors<span id="l1"><h6 style="display:inline"><b> Loading...</b></h6></span></h2>
        <form class="pure-form pure-form-aligned" id='f1'>
            <fieldset>

        <div class="pure-control-group">
            <label for="cr1">Clamp 1 Ratio</label>
            <input type='number' id='cr1' name='cr1' step='0.01'/>
        </div>
        <div class="pure-control-group">
            <label for="cnc1">Clamp 1 Noise Cancellation</label>
            <input type='number' id='cnc1' name='cnc1' step='0.001'/>
        </div>
        <div class="pure-control-group">
            <label for="cr2">Clamp 2 Ratio</label>
            <input type='number' id='cr2' name='cr2' step='0.01'/>
        </div>
        <div class="pure-control-group">
            <label for="cnc2">Clamp 2 Noise Cancellation</label>
            <input type='number' id='cnc2' name='cnc2' step='0.001'/>
        </div>
        <div class="pure-control-group">
            <label for="cr3">Clamp 3 Ratio</label>
            <input type='number' id='cr3' name='cr3' step='0.01'/>
        </div>
        <div class="pure-control-group">
            <label for="cnc3">Clamp 3 Noise Cancellation</label>
            <input type='number' id='cnc3' name='cnc3' step='0.001'/>
        </div>

        <div class="pure-control-group">
            <label for="je" class="pure-checkbox">Jeedom Upload</label>
            <input id='je' name='je' type="checkbox">
        </div>
        <div id='j' style='display:none'>
            <div class="pure-control-group">
                <label for="jt">SSL/TLS</label>
                <input type='checkbox' id='jt' name='jt'>
            </div>
            <div class="pure-control-group">
                <label for="jh">Hostname</label>
                <input type='text' id='jh' name='jh' maxlength='64' pattern='[A-Za-z0-9-.]+' size='50' title='DNS name or IP of the Jeedom server'>
                <span class="pure-form-message-inline">(Jeedom Hostname should match with certificate name if SSL/TLS is enabled)</span>
            </div>
            <div class="pure-control-group">
                <label for="ja">ApiKey</label>
                <input type='password' id='ja' name='ja' maxlength='48' pattern='[A-Za-z0-9-.]+' size=50 title='APIKey from Jeedom configuration webpage'>
            </div>
            <div class="pure-control-group">
                <label for="ct">CommandType</label>
                <input type='text' id='ct' name='ct' maxlength='10'>
                <span class="pure-form-message-inline">(Virtual = 'virtual')</span>
            </div>
            <div class="pure-control-group">
                <label for="c1">Clamp1 Id</label>
                <input type='number' id='c1' name='c1' min='0' max='65535'>
                <span class="pure-form-message-inline">(Jeedom Command Id) (0 means disabled)</span>
            </div>
            <div class="pure-control-group">
                <label for="c2">Clamp2 Id</label>
                <input type='number' id='c2' name='c2' min='0' max='65535'>
            </div>
            <div class="pure-control-group">
                <label for="c3">Clamp3 Id</label>
                <input type='number' id='c3' name='c3' min='0' max='65535'>
            </div>
            <div id='jf'>
                <div class="pure-control-group">
                    <label for="jfp">TLS FingerPrint</label>
                    <input type='text' id='jfp' name='jfp' maxlength='59' pattern='^([0-9A-Fa-f]{2}[ :-]*){19}([0-9A-Fa-f]{2})$' size='65'>
                    <span class="pure-form-message-inline">(separators are : &lt;none&gt;,&lt;space&gt;,:,-)</span>
                </div>
            </div>
        </div>
                <div class="pure-controls">
                    <input type='submit' value='Save' class="pure-button pure-button-primary" disabled>
                </div>
            </fieldset>
        </form>
        <span id='r1'></span>
'@
        ;
        HTMLScript=@'
        function onJEChange(){
            if($("#je").prop("checked")) $("#j").show();
            else $("#j").hide();
        };
        $("#je").change(onJEChange);

        function onJTChange(){
            if($("#jt").prop("checked")) $("#jf").show();
            else $("#jf").hide();
        };
        $("#jt").change(onJTChange);

        $("#f1").submit(function(event){
            $("#r1").html("Saving Configuration...");
            $.post("/sc1",$("#f1").serialize(),function(){ 
                $("#f1").hide();
                var reload5sec=document.createElement('script');
                reload5sec.text='var count=4;var cdi=setInterval(function(){$("#cd").text(count);if(!count){clearInterval(cdi);location.reload();}count--;},1000);';
                $('#r1').html('<h3><b>Configuration saved <span style="color: green;">successfully</span>. System is restarting now.</b></h3>This page will be reloaded in <span id="cd">5</span>sec.').append(reload5sec);
            }).fail(function(){
                $('#r1').html('<h3><b>Configuration <span style="color: red;">error</span>.</b></h3>');
            });
            event.preventDefault();
        });
'@
        ;
        HTMLScriptInReady=@'
        $.getJSON("/gc1", function(GC1){

            $.each(GC1,function(k,v){

                if($('#'+k).prop('type')!='checkbox') $('#'+k).val(v);
                else $('#'+k).prop("checked",v);

                $('#'+k).trigger("change");
            })

            $("input[type=submit]",$("#f1")).prop("disabled",false);
            $("#l1").fadeOut();
        })
        .fail(function(){
            $("#l1").html('<h6 style="display:inline;color:red;"><b> Failed</b></h6>');
        });
'@
    }
    ;
    #---fw.html---
    "fw.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
    }
    ;
    #---discover.html---
    "discover.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
    }
}

#call script that prepare Common Web Files and contain compression/Convert/Merge functions
. ..\src\data\_prepareCommonWebFiles.ps1

$path=(Split-Path -Path $MyInvocation.MyCommand.Path)
$templatePath=($path+"\..\src\data")

Write-Host "--- Prepare Application Web Files ---"
Convert-TemplatesWithCustoToCppHeader -templatePath $templatePath -filesAndCusto $templatesWithCustoFiles -destinationPath $path
Convert-FilesToCppHeader -Path $path -FileNames $specificFiles
Write-Host ""